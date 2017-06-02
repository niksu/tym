/*
 * Translation between clause representations.
 * Nik Sultana, April 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "translate.h"

const char * const eqK = "=";
const char * const distinctK = "distinct";

const struct fmla_t *
translate_atom(const struct atom_t * at)
{
  assert(NULL != at);
  struct term_t ** args = malloc(sizeof(struct term_t *) * at->arity);
  for (int i = 0; i < at->arity; i++) {
    args[i] = copy_term(&(at->args[i]));
  }
  return mk_fmla_atom(at->predicate, at->arity, args);
}

const struct fmla_t *
translate_body(const struct clause_t * cl)
{
  const struct fmlas_t * fmlas = NULL;
  for (int i = 0; i < cl->body_size; i++) {
    fmlas = mk_fmla_cell(translate_atom(cl->body + i), fmlas);
  }
  return mk_fmla_ands(fmlas);
}

struct fmlas_t *
translate_bodies(const struct clauses_t * cls)
{
  const struct clauses_t * cursor = cls;
  struct fmlas_t * fmlas = NULL;
  if (NULL != cursor) {
// FIXME check if the translation involved any sharing (i.e., should the result
//       type by a const?)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    fmlas = (struct fmlas_t *)mk_fmla_cell(translate_body(cursor->clause),
      translate_bodies(cursor->next));
#pragma GCC diagnostic pop
  }
  return fmlas;
}

const struct fmla_t *
translate_valuation(struct valuation_t * const v)
{
  const struct fmlas_t * result = NULL;
  struct valuation_t * cursor = v;
  while (NULL != cursor) {
    result = mk_fmla_cell(mk_fmla_atom_varargs("=", 2, mk_term(VAR, cursor->var), cursor->val), result);
    cursor = cursor->next;
  }
  return mk_fmla_ands(result);
}

const struct fmla_t *
translate_query_fmla_atom(struct model_t * mdl, struct sym_gen_t * cg, const struct fmla_atom_t * at)
{
  const struct fmla_t * result = NULL;
  struct term_t ** args = malloc(sizeof(struct term_t *) * at->arity);
  for (int i = 0; i < at->arity; i++) {
    if (VAR == at->predargs[i]->kind) {
      char * placeholder = mk_new_var(cg);
      args[i] = mk_term(CONST, placeholder);

      struct stmt_t * stmt = mk_stmt_const(placeholder, mdl->universe, universe_ty);
      strengthen_model(mdl, stmt);
    } else {
      args[i] = copy_term(at->predargs[i]);
    }
  }
  result = mk_fmla_atom(at->pred_name, at->arity, args);
  return result;
}

const struct fmla_t *
translate_query_fmla(struct model_t * mdl, struct sym_gen_t * cg, const struct fmla_t * fmla)
{
  const struct fmla_t * result = NULL;
  const struct fmla_t * fmla_2 = NULL;
  const struct fmla_t * fmla_3 = NULL;
  // FIXME free intermediate allocations.

  switch (fmla->kind) {
  case FMLA_CONST:
    result = copy_fmla(fmla);
    break;
  case FMLA_ATOM:
    result = translate_query_fmla_atom(mdl, cg, fmla->param.atom);
    break;
  case FMLA_AND:
    fmla_2 = translate_query_fmla(mdl, cg, fmla->param.args[0]);
    fmla_3 = translate_query_fmla(mdl, cg, fmla->param.args[1]);
    result = mk_fmla_and(fmla_2, fmla_3);
    break;
  case FMLA_OR:
    fmla_2 = translate_query_fmla(mdl, cg, fmla->param.args[0]);
    fmla_3 = translate_query_fmla(mdl, cg, fmla->param.args[1]);
    result = mk_fmla_or(fmla_2, fmla_3);
    break;
  case FMLA_NOT:
    fmla_2 = translate_query_fmla(mdl, cg, fmla->param.args[0]);
    result = mk_fmla_not(fmla_2);
    break;
  case FMLA_EX:
    assert(false); // Existential quantifier cannot appear in queries.
    break;
  default:
    assert(false); // No other formula constructor exists.
    break;
  }

  return result;
}

void
translate_query(struct program_t * query, struct model_t * mdl, struct sym_gen_t * cg)
{
#if DEBUG
  printf("|query|=%d\n", query->no_clauses);
#endif
  // NOTE we expect a query to contain exactly one clause.
  assert(1 == query->no_clauses);
  const struct clause_t * q_cl = query->program[0];

  const struct fmla_t * q_fmla = translate_atom(&(q_cl->head));
  const struct fmla_t * translated_q = translate_query_fmla(mdl, cg, q_fmla);

#if DEBUG
  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = Bfmla_str(q_fmla, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("q_fmla (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  free_buffer(outbuf);
#endif

  const struct stmt_t * stmt = mk_stmt_axiom(translated_q);
  strengthen_model(mdl, stmt);
}

struct model_t *
translate_program(struct program_t * program, struct sym_gen_t ** vg)
{
  struct atom_database_t * adb = mk_atom_database();

  for (int i = 0; i < program->no_clauses; i++) {
    (void)clause_database_add(program->program[i], adb, NULL);
  }
  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = Batom_database_str(adb, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
#if DEBUG
  printf("clause database (remaining=%zu)\n|%s|\n",
      outbuf->buffer_size - outbuf->idx, outbuf->buffer);
#endif


  // 1. Generate prologue: universe sort, and its inhabitants.
  struct model_t * mdl = mk_model(mk_universe(adb->tdb->herbrand_universe));

#if DEBUG
  res = Bmodel_str(mdl, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("model (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
#else
  res = Bmodel_str(mdl, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
#endif


  // 2. Add axiom characterising the provability of all elements of the Hilbert base.
  struct predicates_t * preds_cursor = atom_database_to_predicates(adb);
  while (NULL != preds_cursor) {
#if DEBUG
    printf("no_bodies = %zu\n", num_predicate_bodies(preds_cursor->predicate));
#endif

    struct mutable_fmlas_t * fmlas = (struct mutable_fmlas_t *)translate_bodies(preds_cursor->predicate->bodies);
    struct mutable_fmlas_t * fmlas_cursor = fmlas;

    if (NULL == preds_cursor->predicate->bodies) {
      // "No bodies" means that the atom never appears as the head of a clause.

      struct term_t ** var_args = malloc(sizeof(struct term_t *) * preds_cursor->predicate->arity);

      for (int i = 0; i < preds_cursor->predicate->arity; i++) {
        var_args[i] = mk_term(VAR, mk_new_var(*vg));
      }

      const struct fmla_t * atom = mk_fmla_atom((const char *)preds_cursor->predicate->predicate,
          preds_cursor->predicate->arity, var_args);

      res = Bfmla_str(atom, outbuf);
      assert(is_ok_buffer_write_result(res));
      free(res);
#if DEBUG
      printf("bodyless: %s\n", outbuf->buffer);
#endif

      strengthen_model(mdl,
          mk_stmt_pred((const char *)preds_cursor->predicate->predicate, arguments_of_atom(fmla_as_atom(atom)), mk_fmla_const(false)));

      free_fmla(atom);
    } else {
      const struct clauses_t * body_cursor = preds_cursor->predicate->bodies;
      const struct fmla_t * abs_head_fmla;

      while (NULL != body_cursor) {
#if DEBUG
        printf(">");
#endif

        struct sym_gen_t * vg_copy = copy_sym_gen(*vg);

        const struct atom_t * head_atom = &(body_cursor->clause->head);
        struct term_t ** args = malloc(sizeof(struct term_t *) * head_atom->arity);

        for (int i = 0; i < head_atom->arity; i++) {
          args[i] = copy_term(&(head_atom->args[i]));
        }

        // Abstract the atom's parameters.
        const struct fmla_t * head_fmla = mk_fmla_atom(head_atom->predicate, head_atom->arity, args);

        res = Bfmla_str(head_fmla, outbuf);
        assert(is_ok_buffer_write_result(res));
        free(res);
#if DEBUG
        printf("from: %s\n", outbuf->buffer);
#endif

        struct valuation_t ** v = malloc(sizeof(struct valuation_t *));
        abs_head_fmla = mk_abstract_vars(head_fmla, vg_copy, v);
        res = Bfmla_str(abs_head_fmla, outbuf);
        assert(is_ok_buffer_write_result(res));
        free(res);
#if DEBUG
        printf("to: %s\n", outbuf->buffer);
#endif

        res = Bvaluation_str(*v, outbuf);
        assert(is_ok_buffer_write_result(res));
#if DEBUG
        if (0 == val_of_buffer_write_result(res)) {
          printf("  where: (no substitutions)\n");
        } else {
          printf("  where: %s\n", outbuf->buffer);
        }
#endif
        free(res);

        const struct fmla_t * valuation_fmla = translate_valuation(*v);
        const struct fmla_t * fmla = fmlas_cursor->fmla;
        const struct fmla_t * anded_fmla = mk_fmla_and(fmla, valuation_fmla);
        fmlas_cursor->fmla = copy_fmla(anded_fmla);
        free_fmla(fmla);
        free_fmla(anded_fmla);
        free_fmla(valuation_fmla);
        struct terms_t * ts = filter_var_values(*v);
        fmla = fmlas_cursor->fmla;
        const struct fmla_t * quantified_fmla = mk_fmla_quants(ts, fmla);
        fmlas_cursor->fmla = copy_fmla(quantified_fmla);
        free_fmla(fmla);
        free_fmla(quantified_fmla);

        res = Bfmla_str(fmlas_cursor->fmla, outbuf);
        assert(is_ok_buffer_write_result(res));
        free(res);
#if DEBUG
        printf("  :|%s|\n", outbuf->buffer);
#endif


        free_fmla(head_fmla);
        if (NULL != *v) {
          // i.e., the predicate isn't nullary.
          free_valuation(*v);
        }
        free(v);

        body_cursor = body_cursor->next;
        fmlas_cursor = fmlas_cursor->next;
        if (NULL == body_cursor) {
          struct sym_gen_t * tmp = *vg;
          *vg = vg_copy;
          vg_copy = tmp;
        }
        free_sym_gen(vg_copy);
      }

      const struct fmla_t * fmla = mk_fmla_ors((struct fmlas_t *)fmlas);
      res = Bfmla_str(fmla, outbuf);
      assert(is_ok_buffer_write_result(res));
      free(res);
#if DEBUG
      printf("pre-result: %s\n", outbuf->buffer);
#endif

      struct fmla_atom_t * head = fmla_as_atom(abs_head_fmla);
      strengthen_model(mdl,
          mk_stmt_pred(head->pred_name, arguments_of_atom(head), fmla));
      free_fmla(abs_head_fmla);
    }

    preds_cursor = preds_cursor->next;

#if DEBUG
    printf("\n");
#endif
  }

  free_buffer(outbuf);

  return mdl;
}

// FIXME naive implementation
const struct stmts_t *
order_statements(const struct stmts_t * stmts)
{
  const struct stmts_t * cursor = stmts;
  const struct stmts_t * waiting = NULL;
  const struct stmts_t * result = NULL;

  struct terms_t * declared = NULL;
  declared = mk_term_cell(mk_term(CONST, eqK), declared);
  declared = mk_term_cell(mk_term(CONST, distinctK), declared);

  while (NULL != cursor || NULL != waiting) {

#if DEBUG
  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = NULL;

#if DEBUG
    printf("|declared| = %d\n", len_terms_cell(declared));
#endif
    res = Bterms_to_str(declared, outbuf);
    assert(is_ok_buffer_write_result(res));
    free(res);
#if DEBUG
    printf("declared (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
#endif

    free_buffer(outbuf);
#endif

    if (NULL == cursor && NULL != waiting) {
#if DEBUG
      printf("Making 'waiting' into 'cursor'.\n");
#endif
      cursor = waiting;
      waiting = NULL;
      continue;
    }
#if DEBUG
    else {
      printf("Not making 'waiting' into 'cursor'.\n");
    }
#endif

#if DEBUG
    res = Bstmt_str(cursor->stmt, outbuf);
    assert(is_ok_buffer_write_result(res));
    free(res);
    printf("cursor->stmt (size=%zu, remaining=%zu)\n|%s|\n",
        outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
#endif

    struct term_t * t = new_const_in_stmt(cursor->stmt);
    struct terms_t * term_consts = consts_in_stmt(cursor->stmt);
    if (terms_subsumed_by(declared, term_consts)) {
      if (NULL != t) {
#if DEBUG
        printf("Term subsumption for %s\n", t->identifier);
#endif
        declared = mk_term_cell(t, declared);
      }
#if DEBUG
      else {
        printf("NULL == t\n");
      }
#endif
      result = mk_stmt_cell(cursor->stmt, result);
    } else {
#if DEBUG
      if (NULL != t) {
        printf("NO term subsumption for %s\n", t->identifier);
      }
#endif
      waiting = mk_stmt_cell(cursor->stmt, waiting);
    }

    cursor = cursor->next;
  }

  const struct stmts_t * reversed = reverse_stmts(result);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  free((void *)result);
#pragma GCC diagnostic pop
  return reversed;
}
