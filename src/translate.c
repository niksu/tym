/*
 * Translation between clause representations.
 * Nik Sultana, April 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "translate.h"

struct fmla_t *
translate_atom(const struct Atom * at)
{
  assert(NULL != at);
  struct Term ** args = NULL;
  if (at->arity > 0) {
    args = malloc(sizeof(struct Term *) * at->arity);
    for (int i = 0; i < at->arity; i++) {
      args[i] = copy_term(at->args[i]);
    }
  }
  return mk_fmla_atom(strdup(at->predicate), at->arity, args);
}

struct fmla_t *
translate_body(const struct Clause * cl)
{
  struct fmlas_t * fmlas = NULL;
  for (int i = 0; i < cl->body_size; i++) {
    fmlas = mk_fmla_cell(translate_atom(cl->body[i]), fmlas);
  }
  return mk_fmla_ands(fmlas);
}

struct fmlas_t *
translate_bodies(const struct Clauses * cls)
{
  const struct Clauses * cursor = cls;
  struct fmlas_t * fmlas = NULL;
  while (NULL != cursor) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    fmlas = (struct fmlas_t *)mk_fmla_cell(translate_body(cursor->clause), fmlas);
#pragma GCC diagnostic pop
    cursor = cursor->next;
  }
  return fmlas;
}

struct fmla_t *
translate_valuation(struct valuation_t * const v)
{
  struct fmlas_t * result = NULL;
  struct valuation_t * cursor = v;
  while (NULL != cursor) {
    result = mk_fmla_cell(mk_fmla_atom_varargs(strdup(eqK), 2,
          mk_term(VAR, strdup(cursor->var)),
          copy_term(cursor->val)), result);
    cursor = cursor->next;
  }
  return mk_fmla_ands(result);
}

void
translate_query_fmla_atom(struct model_t * mdl, struct sym_gen_t * cg, struct fmla_atom_t * at)
{
  struct Term ** args = NULL;
  if (at->arity > 0) {
    args = malloc(sizeof(struct Term *) * at->arity);
    for (int i = 0; i < at->arity; i++) {
      if (VAR == at->predargs[i]->kind) {
        char * placeholder = mk_new_var(cg);
        args[i] = mk_term(CONST, placeholder);

        struct stmt_t * stmt = mk_stmt_const(strdup(placeholder), mdl->universe, universe_ty);
        strengthen_model(mdl, stmt);
      } else {
        args[i] = copy_term(at->predargs[i]);
      }
      free_term(at->predargs[i]);
    }
    free(at->predargs);
  }
  at->predargs = args;
}

void
translate_query_fmla(struct model_t * mdl, struct sym_gen_t * cg, struct fmla_t * fmla)
{
  switch (fmla->kind) {
  case FMLA_CONST:
    // Nothing to do
    break;
  case FMLA_ATOM:
    translate_query_fmla_atom(mdl, cg, fmla->param.atom);
    break;
  case FMLA_AND:
    translate_query_fmla(mdl, cg, fmla->param.args[0]);
    translate_query_fmla(mdl, cg, fmla->param.args[1]);
    break;
  case FMLA_OR:
    translate_query_fmla(mdl, cg, fmla->param.args[0]);
    translate_query_fmla(mdl, cg, fmla->param.args[1]);
    break;
  case FMLA_NOT:
    translate_query_fmla(mdl, cg, fmla->param.args[0]);
    break;
  case FMLA_EX:
    assert(false); // Existential quantifier cannot appear in queries.
    break;
  default:
    assert(false); // No other formula constructor exists.
    break;
  }
}

void
translate_query(struct Program * query, struct model_t * mdl, struct sym_gen_t * cg)
{
#if DEBUG
  printf("|query|=%d\n", query->no_clauses);
#endif
  // NOTE we expect a query to contain exactly one clause.
  assert(1 == query->no_clauses);
  const struct Clause * q_cl = query->program[0];

  struct fmla_t * q_fmla = translate_atom(q_cl->head);

#if DEBUG
  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = fmla_str(q_fmla, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("q_fmla (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  free_buffer(outbuf);
#endif

  // Reject the query if it containts constants that don't appear in the program.
  struct Terms * cursor = consts_in_fmla(q_fmla, NULL);
  while (NULL != cursor) {
    if (CONST == cursor->term->kind) {
      bool found = false;
      for (int i = 0; i < mdl->universe->cardinality; i++) {
        if (0 == strcmp(cursor->term->identifier, mdl->universe->element[i])) {
          found = true;
          break;
        }
      }
      if (!found) {
        printf("The constant '%s' in the query doesn't appear in the program.\n",
            cursor->term->identifier);
        assert(false);
      }
      struct Terms * pre_cursor = cursor;
      cursor = cursor->next;
      free(pre_cursor);
    }
  }
  translate_query_fmla(mdl, cg, q_fmla);

  const struct stmt_t * stmt = mk_stmt_axiom(q_fmla);
  strengthen_model(mdl, stmt);
}

struct model_t *
translate_program(struct Program * program, struct sym_gen_t ** vg)
{
  struct atom_database_t * adb = mk_atom_database();

  for (int i = 0; i < program->no_clauses; i++) {
    (void)clause_database_add(program->program[i], adb, NULL);
  }
  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = atom_database_str(adb, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
#if DEBUG
  printf("clause database (remaining=%zu)\n|%s|\n",
      outbuf->buffer_size - outbuf->idx, outbuf->buffer);
#endif


  // 1. Generate prologue: universe sort, and its inhabitants.
  struct model_t * mdl = mk_model(mk_universe(adb->tdb->herbrand_universe));

#if DEBUG
  res = model_str(mdl, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("model (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
#else
  res = model_str(mdl, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
#endif


  // 2. Add axiom characterising the provability of all elements of the Hilbert base.
  struct predicates_t * preds_cursor = atom_database_to_predicates(adb);
  while (NULL != preds_cursor) {
#if DEBUG
    printf("no_bodies = %zu\n", num_predicate_bodies(preds_cursor->predicate));
#endif

    struct fmlas_t * fmlas = (struct fmlas_t *)translate_bodies(preds_cursor->predicate->bodies);
    struct fmlas_t * fmlas_cursor = fmlas;

    if (NULL == preds_cursor->predicate->bodies) {
      // "No bodies" means that the atom never appears as the head of a clause.

      struct Term ** var_args = NULL;

      if (preds_cursor->predicate->arity > 0) {
        var_args = malloc(sizeof(struct Term *) * preds_cursor->predicate->arity);

        for (int i = 0; i < preds_cursor->predicate->arity; i++) {
          var_args[i] = mk_term(VAR, mk_new_var(*vg));
        }
      }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
      const struct fmla_t * atom =
        mk_fmla_atom(strdup(preds_cursor->predicate->predicate),
          preds_cursor->predicate->arity, var_args);
#pragma GCC diagnostic pop

      res = fmla_str(atom, outbuf);
      assert(is_ok_buffer_write_result(res));
      free(res);
#if DEBUG
      printf("bodyless: %s\n", outbuf->buffer);
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
      strengthen_model(mdl,
          mk_stmt_pred(strdup(preds_cursor->predicate->predicate),
            arguments_of_atom(fmla_as_atom(atom)),
            mk_fmla_const(false)));
#pragma GCC diagnostic pop

      free_fmla(atom);
    } else {
      const struct Clauses * body_cursor = preds_cursor->predicate->bodies;
      const struct fmla_t * abs_head_fmla = NULL;

      while (NULL != body_cursor) {
#if DEBUG
        printf(">");
#endif

        struct sym_gen_t * vg_copy = copy_sym_gen(*vg);

        const struct Atom * head_atom = body_cursor->clause->head;
        struct Term ** args = NULL;

        if (head_atom->arity > 0) {
          args = malloc(sizeof(struct Term *) * head_atom->arity);

          for (int i = 0; i < head_atom->arity; i++) {
            args[i] = copy_term(head_atom->args[i]);
          }
        }

        // Abstract the atom's parameters.
        const struct fmla_t * head_fmla =
          mk_fmla_atom(strdup(head_atom->predicate), head_atom->arity, args);

        res = fmla_str(head_fmla, outbuf);
        assert(is_ok_buffer_write_result(res));
        free(res);
#if DEBUG
        printf("from: %s\n", outbuf->buffer);
#endif

        struct valuation_t ** val = malloc(sizeof(struct valuation_t *));
        *val = NULL;
        if (NULL != abs_head_fmla) {
          free_fmla(abs_head_fmla);
        }
        abs_head_fmla = mk_abstract_vars(head_fmla, vg_copy, val);
        res = fmla_str(abs_head_fmla, outbuf);
        assert(is_ok_buffer_write_result(res));
        free(res);
#if DEBUG
        printf("to: %s\n", outbuf->buffer);
#endif

        res = valuation_str(*val, outbuf);
        assert(is_ok_buffer_write_result(res));
#if DEBUG
        if (0 == val_of_buffer_write_result(res)) {
          printf("  where: (no substitutions)\n");
        } else {
          printf("  where: %s\n", outbuf->buffer);
        }
#endif
        free(res);

        struct fmla_t * valuation_fmla = translate_valuation(*val);
        fmlas_cursor->fmla = mk_fmla_and(fmlas_cursor->fmla, valuation_fmla);
        struct Terms * ts = filter_var_values(*val);
        const struct fmla_t * quantified_fmla =
          mk_fmla_quants(ts, fmlas_cursor->fmla);
        fmlas_cursor->fmla = copy_fmla(quantified_fmla);
        if (NULL != ts) {
          free_terms(ts);
        }
        free_fmla(quantified_fmla);

        res = fmla_str(fmlas_cursor->fmla, outbuf);
        assert(is_ok_buffer_write_result(res));
        free(res);
#if DEBUG
        printf("  :|%s|\n", outbuf->buffer);
#endif

        free_fmla(head_fmla);
        if (NULL != *val) {
          // i.e., the predicate isn't nullary.
          free_valuation(*val);
        }
        free(val);

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
      res = fmla_str(fmla, outbuf);
      assert(is_ok_buffer_write_result(res));
      free(res);
#if DEBUG
      printf("pre-result: %s\n", outbuf->buffer);
#endif

      struct fmla_atom_t * head = fmla_as_atom(abs_head_fmla);
      strengthen_model(mdl,
          mk_stmt_pred(strdup(head->pred_name),
            arguments_of_atom(head),
            fmla));
      free_fmla(abs_head_fmla);
    }


    struct predicates_t * pre_preds_cursor = preds_cursor;
    preds_cursor = preds_cursor->next;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    free((void *)pre_preds_cursor);
#pragma GCC diagnostic pop

#if DEBUG
    printf("\n");
#endif
  }

  free_buffer(outbuf);

  free_atom_database(adb);

  return mdl;
}

// FIXME naive implementation
const struct stmts_t *
order_statements(const struct stmts_t * stmts)
{
  const struct stmts_t * cursor = stmts;
  const struct stmts_t * waiting = NULL;
  const struct stmts_t * result = NULL;

  struct Terms * declared = NULL;
  declared = mk_term_cell(mk_term(CONST, strdup(eqK)), declared);
  declared = mk_term_cell(mk_term(CONST, strdup(distinctK)), declared);

  bool cursor_is_waiting = false;

  while (NULL != cursor || NULL != waiting) {

#if DEBUG
    struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
    struct buffer_write_result * res = NULL;

    printf("|declared| = %d\n", len_Terms_cell(declared));

    res = terms_to_str(declared, outbuf);
    assert(is_ok_buffer_write_result(res));
    free(res);

    printf("declared (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
    free_buffer(outbuf);
#endif

    if (NULL == cursor && NULL != waiting) {
#if DEBUG
      printf("Making 'waiting' into 'cursor'.\n");
#endif
      cursor = waiting;
      waiting = NULL;
      cursor_is_waiting = true;
      continue;
    }
#if DEBUG
    else {
      printf("Not making 'waiting' into 'cursor'.\n");
    }
#endif

#if DEBUG
    outbuf = mk_buffer(BUF_SIZE);
    res = stmt_str(cursor->stmt, outbuf);
    assert(is_ok_buffer_write_result(res));
    free(res);
    printf("cursor->stmt (size=%zu, remaining=%zu)\n|%s|\n",
        outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);

    free_buffer(outbuf);
#endif

    struct Term * t = new_const_in_stmt(cursor->stmt);
    struct Terms * term_consts = consts_in_stmt(cursor->stmt);
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    struct Terms * pre_term_consts = NULL;
    while (NULL != term_consts) {
      pre_term_consts = term_consts;
      term_consts = term_consts->next;
      free((void *)pre_term_consts);
    }

    const struct stmts_t * pre_cursor = cursor;
    cursor = cursor->next;
    if (cursor_is_waiting) {
      free((void *)pre_cursor);
    }
#pragma GCC diagnostic pop
  }

  const struct stmts_t * reversed = reverse_stmts(result);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  const struct stmts_t * pre_cursor = NULL;
  cursor = result;
  while (NULL != cursor) {
    pre_cursor = cursor;
    cursor = cursor->next;
    free((void *)pre_cursor);
  }
#pragma GCC diagnostic pop
  free_terms(declared);
  return reversed;
}
