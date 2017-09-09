/*
 * Translation between clause representations.
 * Nik Sultana, April 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "translate.h"

struct TymFmla *
tym_translate_atom(const struct TymAtom * at)
{
  assert(NULL != at);
  struct TymTerm ** args = NULL;
  if (at->arity > 0) {
    args = malloc(sizeof *args * at->arity);
    for (int i = 0; i < at->arity; i++) {
      args[i] = tym_copy_term(at->args[i]);
    }
  }
  return tym_mk_fmla_atom(TYM_STR_DUPLICATE(at->predicate), at->arity, args);
}

struct TymFmla *
tym_translate_body(const struct TymClause * cl)
{
  struct TymFmlas * fmlas = NULL;
  for (int i = 0; i < cl->body_size; i++) {
    fmlas = tym_mk_fmla_cell(tym_translate_atom(cl->body[i]), fmlas);
  }
  return tym_mk_fmla_ands(fmlas);
}

struct TymFmlas *
tym_translate_bodies(const struct TymClauses * cls)
{
  const struct TymClauses * cursor = cls;
  struct TymFmlas * result = NULL;
  struct TymFmlas * result_end = NULL;
  while (NULL != cursor) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    if (NULL == result && NULL == result_end) {
      result = (struct TymFmlas *)tym_mk_fmla_cell(tym_translate_body(cursor->clause), NULL);
      result_end = result;
    } else {
      result_end->next = (struct TymFmlas *)tym_mk_fmla_cell(tym_translate_body(cursor->clause), NULL);
      result_end = result_end->next;
    }
#pragma GCC diagnostic pop
    cursor = cursor->next;
  }
  return result;
}

struct TymFmla *
tym_translate_valuation(struct TymValuation * const v)
{
  struct TymFmlas * result = NULL;
  struct TymValuation * cursor = v;
  while (NULL != cursor) {
    result = tym_mk_fmla_cell(tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE(tym_eqK), 2,
          tym_mk_term(TYM_VAR, TYM_STR_DUPLICATE(cursor->var)),
          tym_copy_term(cursor->val)), result);
    cursor = cursor->next;
  }
  return tym_mk_fmla_ands(result);
}

void
tym_translate_query_fmla_atom(struct TymModel * mdl, struct TymSymGen * cg, struct TymFmlaAtom * at)
{
  struct TymTerm ** args = NULL;
  if (at->arity > 0) {
    args = malloc(sizeof *args * at->arity);
    for (int i = 0; i < at->arity; i++) {
      if (TYM_VAR == at->predargs[i]->kind) {
        const TymStr * placeholder = tym_mk_new_var(cg);
        args[i] = tym_mk_term(TYM_CONST, placeholder);

        struct TymStmt * stmt =
          tym_mk_stmt_const(TYM_STR_DUPLICATE(placeholder),
              mdl->universe, TYM_CSTR_DUPLICATE(TYM_UNIVERSE_TY));
        tym_strengthen_model(mdl, stmt);
      } else {
        args[i] = tym_copy_term(at->predargs[i]);
      }
      tym_free_term(at->predargs[i]);
    }
    free(at->predargs);
  }
  at->predargs = args;
}

void
tym_translate_query_fmla(struct TymModel * mdl, struct TymSymGen * cg, struct TymFmla * fmla)
{
  switch (fmla->kind) {
  case FMLA_CONST:
    // Nothing to do
    break;
  case FMLA_ATOM:
    tym_translate_query_fmla_atom(mdl, cg, fmla->param.atom);
    break;
  case FMLA_AND:
    tym_translate_query_fmla(mdl, cg, fmla->param.args[0]);
    tym_translate_query_fmla(mdl, cg, fmla->param.args[1]);
    break;
  case FMLA_OR:
    tym_translate_query_fmla(mdl, cg, fmla->param.args[0]);
    tym_translate_query_fmla(mdl, cg, fmla->param.args[1]);
    break;
  case FMLA_NOT:
    tym_translate_query_fmla(mdl, cg, fmla->param.args[0]);
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
tym_translate_query(struct TymProgram * query, struct TymModel * mdl, struct TymSymGen * cg)
{
#if TYM_DEBUG
  printf("|query|=%d\n", query->no_clauses);
#endif
  // NOTE we expect a query to contain exactly one clause.
  assert(1 == query->no_clauses);
  const struct TymClause * q_cl = query->program[0];

  struct TymFmla * q_fmla = tym_translate_atom(q_cl->head);

#if TYM_DEBUG
  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TymLiftedTymBufferWriteResult * res = tym_fmla_str(q_fmla, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  printf("q_fmla (size=%zu, remaining=%zu)\n|%s|\n",
    tym_buffer_len(outbuf),
    tym_buffer_size(outbuf) - tym_buffer_len(outbuf),
    tym_buffer_contents(outbuf));
  tym_free_buffer(outbuf);
#endif

  // Reject the query if it containts constants that don't appear in the program.
  struct TymTerms * cursor = tym_consts_in_fmla(q_fmla, NULL);
  while (NULL != cursor) {
    if (TYM_CONST == cursor->term->kind) {
      bool found = false;
      for (int i = 0; i < mdl->universe->cardinality; i++) {
        if (0 == tym_cmp_str(cursor->term->identifier, mdl->universe->element[i])) {
          found = true;
          break;
        }
      }
      if (!found) {
        printf("The constant '%s' in the query doesn't appear in the program.\n",
            tym_decode_str(cursor->term->identifier));
        assert(false);
      }
      struct TymTerms * pre_cursor = cursor;
      cursor = cursor->next;
      free(pre_cursor);
    }
  }
  tym_translate_query_fmla(mdl, cg, q_fmla);

  const struct TymStmt * stmt = tym_mk_stmt_axiom(q_fmla);
  tym_strengthen_model(mdl, stmt);
}

struct TymModel *
tym_translate_program(struct TymProgram * program, struct TymSymGen ** vg)
{
  struct TymAtomDatabase * adb = tym_mk_atom_database();

  for (int i = 0; i < program->no_clauses; i++) {
    (void)tym_clause_database_add(program->program[i], adb, NULL);
  }
  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_atom_database_str(adb, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
#if TYM_DEBUG
  printf("clause database (size=%lu, remaining=%zu)\n|%s|\n",
      tym_buffer_len(outbuf),
      tym_buffer_size(outbuf) - tym_buffer_len(outbuf),
      tym_buffer_contents(outbuf));
#endif


  // 1. Generate prologue: universe sort, and its inhabitants.
  struct TymModel * mdl = tym_mk_model(tym_mk_universe(adb->tdb->herbrand_universe));

#if TYM_DEBUG
  res = tym_model_str(mdl, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  printf("model (size=%zu, remaining=%zu)\n|%s|\n",
      tym_buffer_len(outbuf),
      tym_buffer_size(outbuf) - tym_buffer_len(outbuf),
      tym_buffer_contents(outbuf));
#else
  res = tym_model_str(mdl, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
#endif


  // 2. Add axiom characterising the provability of all elements of the Hilbert base.
  struct TymPredicates * preds_cursor = tym_atom_database_to_predicates(adb);
  while (NULL != preds_cursor) {
#if TYM_DEBUG
    printf("no_bodies = %zu\n", tym_num_predicate_bodies(preds_cursor->predicate));
#endif

    struct TymFmlas * fmlas = (struct TymFmlas *)tym_translate_bodies(preds_cursor->predicate->bodies);
#if TYM_DEBUG
    tym_reset_buffer(outbuf);
    struct TymFmlas * fmlas_c = fmlas;
    while (NULL != fmlas_c) {
      res = tym_fmla_str(fmlas_c->fmla, outbuf);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
      fmlas_c = fmlas_c->next;
    }
    printf(">-: %s\n", tym_buffer_contents(outbuf));
#endif

    struct TymFmlas * fmlas_cursor = fmlas;

    if (NULL == preds_cursor->predicate->bodies) {
      // "No bodies" means that the atom never appears as the head of a clause.

      struct TymTerm ** var_args = NULL;

      if (preds_cursor->predicate->arity > 0) {
        var_args = malloc(sizeof *var_args * preds_cursor->predicate->arity);

        for (int i = 0; i < preds_cursor->predicate->arity; i++) {
          var_args[i] = tym_mk_term(TYM_VAR, tym_mk_new_var(*vg));
        }
      }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
      const struct TymFmla * atom =
        tym_mk_fmla_atom(TYM_STR_DUPLICATE(preds_cursor->predicate->predicate),
          preds_cursor->predicate->arity, var_args);
#pragma GCC diagnostic pop

      res = tym_fmla_str(atom, outbuf);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
#if TYM_DEBUG
      printf("bodyless: %s\n", tym_buffer_contents(outbuf));
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
      tym_strengthen_model(mdl,
          tym_mk_stmt_pred(TYM_STR_DUPLICATE(preds_cursor->predicate->predicate),
            tym_arguments_of_atom(tym_fmla_as_atom(atom)),
            tym_mk_fmla_const(false)));
#pragma GCC diagnostic pop

      tym_free_fmla(atom);
    } else {
      const struct TymClauses * body_cursor = preds_cursor->predicate->bodies;
      const struct TymFmla * abs_head_fmla = NULL;

      while (NULL != body_cursor) {
#if TYM_DEBUG
        printf(">");
#endif

        struct TymSymGen * vg_copy = tym_copy_sym_gen(*vg);

        const struct TymAtom * head_atom = body_cursor->clause->head;
        struct TymTerm ** args = NULL;

        if (head_atom->arity > 0) {
          args = malloc(sizeof *args * head_atom->arity);

          for (int i = 0; i < head_atom->arity; i++) {
            args[i] = tym_copy_term(head_atom->args[i]);
          }
        }

        // Abstract the atom's parameters.
        const struct TymFmla * head_fmla =
          tym_mk_fmla_atom(TYM_STR_DUPLICATE(head_atom->predicate),
              head_atom->arity, args);

#if TYM_DEBUG // FIXME cover more code using the #if?
        res = tym_fmla_str(head_fmla, outbuf);
        assert(tym_is_ok_TymBufferWriteResult(res));
        free(res);
        printf("from: %s\n", tym_buffer_contents(outbuf));
#endif

        struct TymValuation ** val = malloc(sizeof *val);
        *val = NULL;
        if (NULL != abs_head_fmla) {
          tym_free_fmla(abs_head_fmla);
        }
        abs_head_fmla = tym_mk_abstract_vars(head_fmla, vg_copy, val);
        res = tym_fmla_str(abs_head_fmla, outbuf);
        assert(tym_is_ok_TymBufferWriteResult(res));
        free(res);
#if TYM_DEBUG
        printf("to: %s\n", tym_buffer_contents(outbuf));
#endif

        res = tym_valuation_str(*val, outbuf);
        assert(tym_is_ok_TymBufferWriteResult(res));
#if TYM_DEBUG
        if (0 == tym_val_of_TymBufferWriteResult(res)) {
          printf("  where: (no substitutions)\n");
        } else {
          printf("  where: %s\n", tym_buffer_contents(outbuf));
        }
#endif
        free(res);

        struct TymFmla * valuation_fmla = tym_translate_valuation(*val);
        fmlas_cursor->fmla = tym_mk_fmla_and(fmlas_cursor->fmla, valuation_fmla);
        struct TymTerms * ts = tym_filter_var_values(*val);
        const struct TymFmla * quantified_fmla =
          tym_mk_fmla_quants(ts, fmlas_cursor->fmla);
        fmlas_cursor->fmla = tym_copy_fmla(quantified_fmla); // FIXME is the copying redundant?
        if (NULL != ts) {
          tym_free_terms(ts);
        }
        tym_free_fmla(quantified_fmla);

        res = tym_fmla_str(fmlas_cursor->fmla, outbuf);
        assert(tym_is_ok_TymBufferWriteResult(res));
        free(res);
#if TYM_DEBUG
        printf("  :|%s|\n", tym_buffer_contents(outbuf));
#endif

        tym_free_fmla(head_fmla);
        if (NULL != *val) {
          // i.e., the predicate isn't nullary.
          tym_free_valuation(*val);
        }
        free(val);

        body_cursor = body_cursor->next;
        fmlas_cursor = fmlas_cursor->next;
        if (NULL == body_cursor) {
          struct TymSymGen * tmp = *vg;
          *vg = vg_copy;
          vg_copy = tmp;
        }
        tym_free_sym_gen(vg_copy);
      }

      struct TymFmla * fmla = tym_mk_fmla_ors((struct TymFmlas *)fmlas);
      res = tym_fmla_str(fmla, outbuf);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
#if TYM_DEBUG
      printf("pre-result: %s\n", tym_buffer_contents(outbuf));
#endif

      struct TymFmlaAtom * head = tym_fmla_as_atom(abs_head_fmla);
      tym_strengthen_model(mdl,
          tym_mk_stmt_pred(TYM_STR_DUPLICATE(head->pred_name),
            tym_arguments_of_atom(head),
            fmla));
      tym_free_fmla(abs_head_fmla);
    }


    struct TymPredicates * pre_preds_cursor = preds_cursor;
    preds_cursor = preds_cursor->next;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    free((void *)pre_preds_cursor);
#pragma GCC diagnostic pop

#if TYM_DEBUG
    printf("\n");
#endif
  }

  tym_free_buffer(outbuf);

  tym_free_atom_database(adb);

  return mdl;
}

// FIXME naive implementation
const struct TymStmts *
tym_order_statements(const struct TymStmts * stmts)
{
  const struct TymStmts * cursor = stmts;
  const struct TymStmts * waiting = NULL;
  const struct TymStmts * result = NULL;

  struct TymTerms * declared = NULL;
  declared = tym_mk_term_cell(tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE(tym_eqK)), declared);
  declared = tym_mk_term_cell(tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE(tym_distinctK)), declared);

  bool cursor_is_waiting = false;

  while (NULL != cursor || NULL != waiting) {

#if TYM_DEBUG
    struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
    struct TymLiftedTymBufferWriteResult * res = NULL;

    printf("|declared| = %d\n", tym_len_TymTerms_cell(declared));

    res = tym_terms_to_str(declared, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    printf("declared (size=%zu, remaining=%zu)\n|%s|\n",
      tym_buffer_len(outbuf),
      tym_buffer_size(outbuf) - tym_buffer_len(outbuf),
      tym_buffer_contents(outbuf));
    tym_free_buffer(outbuf);
#endif

    if (NULL == cursor && NULL != waiting) {
#if TYM_DEBUG
      printf("Making 'waiting' into 'cursor'.\n");
#endif
      cursor = waiting;
      waiting = NULL;
      cursor_is_waiting = true;
      continue;
    }
#if TYM_DEBUG
    else {
      printf("Not making 'waiting' into 'cursor'.\n");
    }
#endif

#if TYM_DEBUG
    outbuf = tym_mk_buffer(TYM_BUF_SIZE);
    res = tym_stmt_str(cursor->stmt, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("cursor->stmt (size=%zu, remaining=%zu)\n|%s|\n",
        tym_buffer_len(outbuf),
        tym_buffer_size(outbuf) - tym_buffer_len(outbuf),
        tym_buffer_contents(outbuf));

    tym_free_buffer(outbuf);
#endif

    struct TymTerm * t = tym_new_const_in_stmt(cursor->stmt);
    struct TymTerms * term_consts = tym_consts_in_stmt(cursor->stmt);
    if (tym_terms_subsumed_by(declared, term_consts)) {
      if (NULL != t) {
#if TYM_DEBUG
        printf("Term subsumption for %s\n", tym_decode_str(t->identifier));
#endif
        declared = tym_mk_term_cell(t, declared);
      }
#if TYM_DEBUG
      else {
        printf("NULL == t\n");
      }
#endif
      result = tym_mk_stmt_cell(cursor->stmt, result);
    } else {
      if (NULL != t) {
#if TYM_DEBUG
        printf("NO term subsumption for %s\n", tym_decode_str(t->identifier));
#endif
        tym_free_term(t);
      }
      waiting = tym_mk_stmt_cell(cursor->stmt, waiting);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    struct TymTerms * pre_term_consts = NULL;
    while (NULL != term_consts) {
      pre_term_consts = term_consts;
      term_consts = term_consts->next;
      free((void *)pre_term_consts);
    }

    const struct TymStmts * pre_cursor = cursor;
    cursor = cursor->next;
    if (cursor_is_waiting) {
      free((void *)pre_cursor);
    }
#pragma GCC diagnostic pop
  }

  const struct TymStmts * reversed = tym_reverse_stmts(result);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  const struct TymStmts * pre_cursor = NULL;
  cursor = result;
  while (NULL != cursor) {
    pre_cursor = cursor;
    cursor = cursor->next;
    free((void *)pre_cursor);
  }
#pragma GCC diagnostic pop
  tym_free_terms(declared);
  return reversed;
}
