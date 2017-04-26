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
translate_atom(struct atom_t * at)
{
  assert(NULL != at);
  struct term_t ** args = malloc(sizeof(struct term_t *) * at->arity);
  for (int i = 0; i < at->arity; i++) {
    args[i] = copy_term(&(at->args[i]));
  }
  return mk_fmla_atom(at->predicate, at->arity, args);
}

struct fmla_t *
translate_body(struct clause_t * cl)
{
  struct fmlas_t * fmlas = NULL;
  for (int i = 0; i < cl->body_size; i++) {
    fmlas = mk_fmla_cell(translate_atom(cl->body + i), fmlas);
  }
  return mk_fmla_ands(fmlas);
}

struct fmlas_t *
translate_bodies(struct clauses_t * cls)
{
  struct clauses_t * cursor = cls;
  struct fmlas_t * fmlas = NULL;
  if (NULL != cursor) {
    fmlas = mk_fmla_cell(translate_body(cursor->clause),
      translate_bodies(cursor->next));
  }
  return fmlas;
}

struct fmla_t *
translate_valuation(struct valuation_t * const v)
{
  struct fmlas_t * result = NULL;
  struct valuation_t * cursor = v;
  while (NULL != cursor) {
    result = mk_fmla_cell(mk_fmla_atom_varargs("=", 2, mk_term(VAR, cursor->var), cursor->val), result);
    cursor = cursor->next;
  }
  return mk_fmla_ands(result);
}

struct fmla_t *
translate_query_fmla_atom(struct model_t * mdl, struct sym_gen_t * cg, const struct fmla_atom_t * at)
{
  struct fmla_t * result = NULL;
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

struct fmla_t *
translate_query_fmla(struct model_t * mdl, struct sym_gen_t * cg, const struct fmla_t * fmla)
{
  struct fmla_t * result = NULL;
  struct fmla_t * fmla_2 = NULL;
  struct fmla_t * fmla_3 = NULL;
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
    // FIXME cannot appear in queries -- complain.
    break;
  default:
    // FIXME fail
    break;
  }

  return result;
}

void
translate_query(struct program_t * query, struct model_t * mdl, struct sym_gen_t * cg)
{
  printf("|query|=%d\n", query->no_clauses);
  struct clause_t * q_cl = query/* FIXME implicit arg8? */->program[0]; // FIXME hardcoding
  struct fmla_t * q_fmla = translate_atom(&(q_cl->head));
  struct fmla_t * translated_q = translate_query_fmla(mdl, cg, q_fmla);

  size_t remaining_buf_size = BUF_SIZE;
  char * buf = malloc(remaining_buf_size);
  size_t l = fmla_str(q_fmla, &remaining_buf_size, buf);
  printf("q_fmla (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);
  l = fmla_str(translated_q, &remaining_buf_size, buf);
  printf("translated_q (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  struct stmt_t * stmt = mk_stmt_axiom(translated_q);
  strengthen_model(mdl, stmt);
}

struct model_t *
translate_program(struct program_t * program, struct sym_gen_t * vg)
{
  struct atom_database_t * adb = mk_atom_database();

  for (int i = 0; i < program->no_clauses; i++) {
    (void)clause_database_add(program->program[i], adb, NULL);
  }
  size_t remaining_buf_size = BUF_SIZE;
  char * buf = malloc(remaining_buf_size);
  atom_database_str(adb, &remaining_buf_size, buf);
  printf("clause database (remaining=%zu)\n|%s|\n", remaining_buf_size, buf);


  // 1. Generate prologue: universe sort, and its inhabitants.
  struct model_t * mdl = mk_model(mk_universe(adb->tdb->herbrand_universe));

  remaining_buf_size = BUF_SIZE;
  size_t l = model_str(mdl, &remaining_buf_size, buf);
  printf("model (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  // NOTE if we don't do this, remaining_buf_size will become 0 causing some
  //      output to be dropped, then it might wrap back and output will resume,
  //      so best to keep it topped up.
  // FIXME maybe should simply remove this parameter then, if it needs maintenance and doesn't yield obvious gain?
  remaining_buf_size = BUF_SIZE;

  // 2. Add axiom characterising the provability of all elements of the Hilbert base.
  struct predicates_t * preds_cursor = atom_database_to_predicates(adb);
  while (NULL != preds_cursor) {
    printf("no_bodies = %zu\n", num_predicate_bodies(preds_cursor->predicate));

    size_t out_size;

    struct fmlas_t * fmlas = translate_bodies(preds_cursor->predicate->bodies);
    struct fmlas_t * fmlas_cursor = fmlas;

    if (NULL == preds_cursor->predicate->bodies) {
      // "No bodies" means that the atom never appears as the head of a clause.

      struct term_t ** var_args = malloc(sizeof(struct term_t *) * preds_cursor->predicate->arity);

      for (int i = 0; i < preds_cursor->predicate->arity; i++) {
        var_args[i] = mk_term(VAR, mk_new_var(vg));
      }

      struct fmla_t * atom = mk_fmla_atom((char *)preds_cursor->predicate->predicate,
          preds_cursor->predicate->arity, var_args);

      out_size = fmla_str(atom, &remaining_buf_size, buf);
      assert(out_size > 0);
      printf("bodyless: %s\n", buf);

      strengthen_model(mdl,
          mk_stmt_pred((char *)preds_cursor->predicate->predicate, arguments_of_atom(fmla_as_atom(atom)), mk_fmla_const(false)));

      free_fmla(atom);
    } else {
      struct clauses_t * body_cursor = preds_cursor->predicate->bodies;
      struct fmla_t * abs_head_fmla;

      while (NULL != body_cursor) {
        printf(">");

        struct sym_gen_t * vg_copy = copy_sym_gen(vg);

        struct atom_t * head_atom = &(body_cursor->clause->head);
        struct term_t ** args = malloc(sizeof(struct term_t *) * head_atom->arity);

        for (int i = 0; i < head_atom->arity; i++) {
          args[i] = copy_term(&(head_atom->args[i]));
        }

        // Abstract the atom's parameters.
        struct fmla_t * head_fmla = mk_fmla_atom(head_atom->predicate, head_atom->arity, args);

        out_size = fmla_str(head_fmla, &remaining_buf_size, buf);
        assert(out_size > 0);
        printf("from: %s\n", buf);

        struct valuation_t ** v = malloc(sizeof(struct valuation_t **));
        abs_head_fmla = mk_abstract_vars(head_fmla, vg_copy, v);
        out_size = fmla_str(abs_head_fmla, &remaining_buf_size, buf);
        assert(out_size > 0);
        printf("to: %s\n", buf);

        out_size = valuation_str(*v, &remaining_buf_size, buf);
        if (0 == out_size) {
          printf("  where: (no substitutions)\n");
        } else {
          printf("  where: %s\n", buf);
        }

        struct fmla_t * valuation_fmla = translate_valuation(*v);
        struct fmla_t * fmla = fmlas_cursor->fmla;
        fmlas_cursor->fmla = mk_fmla_and(fmla, valuation_fmla);
        free_fmla(fmla);
        free_fmla(valuation_fmla);
        struct terms_t * ts = filter_var_values(*v);
        fmla = fmlas_cursor->fmla;
        fmlas_cursor->fmla = mk_fmla_quants(ts, fmla);
        free_fmla(fmla);

        remaining_buf_size = BUF_SIZE;
        fmla_str(fmlas_cursor->fmla, &remaining_buf_size, buf);
        printf("  :|%s|\n", buf);


        free_fmla(head_fmla);
        if (NULL != *v) {
          // i.e., the predicate isn't nullary.
          free_valuation(*v);
        }
        free(v);

        body_cursor = body_cursor->next;
        fmlas_cursor = fmlas_cursor->next;
        if (NULL == body_cursor) {
          struct sym_gen_t * tmp = vg;
          vg = vg_copy;
          vg_copy = tmp;
        }
        free_sym_gen(vg_copy);
      }

      struct fmla_t * fmla = mk_fmla_ors(fmlas);
      remaining_buf_size = BUF_SIZE;
      out_size = fmla_str(fmla, &remaining_buf_size, buf);
      assert(out_size > 0);
      printf("pre-result: %s\n", buf);

      struct fmla_atom_t * head = fmla_as_atom(abs_head_fmla);
      strengthen_model(mdl,
          mk_stmt_pred(head->pred_name, arguments_of_atom(head), fmla));
      free_fmla(abs_head_fmla);
    }

    preds_cursor = preds_cursor->next;

    printf("\n");
  }

  free(buf);

  return mdl;
}

// FIXME naive implementation
struct stmts_t *
order_statements(struct stmts_t * stmts)
{
  struct stmts_t * cursor = stmts;
  struct stmts_t * waiting = NULL;
  struct stmts_t * result = NULL;

  struct terms_t * declared = NULL;
  declared = mk_term_cell(mk_term(CONST, "="/* FIXME const */), declared);
  declared = mk_term_cell(mk_term(CONST, "distinct"/* FIXME const */), declared);

  while (NULL != cursor || NULL != waiting) {

#if DEBUG
    size_t remaining_buf_size = BUF_SIZE;
    char * buf = malloc(remaining_buf_size);
    size_t l;

#if 0
    remaining_buf_size = BUF_SIZE;
    l = stmts_str(cursor, &remaining_buf_size, buf);
    printf("cursor (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

    remaining_buf_size = BUF_SIZE;
    l = stmts_str(waiting, &remaining_buf_size, buf);
    printf("waiting (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);
#endif

    printf("|declared| = %d\n", len_term_cell(declared));
    remaining_buf_size = BUF_SIZE;
    l = terms_to_str(declared, &remaining_buf_size, buf);
    printf("declared (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

    free(buf);
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
    remaining_buf_size = BUF_SIZE;
    l = stmt_str(cursor->stmt, &remaining_buf_size, buf);
    printf("cursor->stmt (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);
#endif

    struct term_t * t = new_const_in_stmt(cursor->stmt);
    struct terms_t * term_consts = consts_in_stmt(cursor->stmt);
    if (terms_subsumed_by(declared, term_consts)) { // FIXME should swap params?
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
  free(stmts);
  return reverse_stmts(result);
}
