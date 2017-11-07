/*
 * Representation of statements (that affect logical models).
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "formula.h"
#include "module_tests.h"
#include "statement.h"
#include "util.h"

char * tym_bool_ty = "Bool";
char * tym_distinctK = "distinct";
char * tym_eqK = "=";

struct TymUniverse *
tym_mk_universe(struct TymTerms * terms)
{
  struct TymUniverse * result = malloc(sizeof *result);
  result->cardinality = 0;
  result->element = NULL;

  const struct TymTerms * cursor = terms;
  while (NULL != cursor) {
    result->cardinality++;
    assert(TYM_CONST == cursor->term->kind);
    cursor = cursor->next;
  }

  if (result->cardinality > 0) {
    result->element = malloc(sizeof *result->element * result->cardinality);
  }

  cursor = terms;
  for (int i = 0; i < result->cardinality; i++) {
    result->element[i] = TYM_STR_DUPLICATE(cursor->term->identifier);
    cursor = cursor->next;
  }

  return result;
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_universe_str(const struct TymUniverse * const uni, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  for (int i = 0; i < uni->cardinality; i++) {
    res = tym_buf_strcpy(dst, "(declare-const");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

    res = tym_buf_strcpy(dst, tym_decode_str(uni->element[i]));
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

    res = tym_buf_strcpy(dst, TYM_UNIVERSE_TY);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.

    if (tym_have_space(dst, 1)) {
      tym_unsafe_buffer_char(dst, '\n');
    } else {
      return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
    }
  }

  res = tym_buf_strcpy(dst, "(assert (distinct");
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  for (int i = 0; i < uni->cardinality; i++) {
    res = tym_buf_strcpy(dst, tym_decode_str(uni->element[i]));
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    if (i < uni->cardinality - 1) {
      tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    } else {
      tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    }
  }

  if (tym_have_space(dst, 3)) {
    tym_unsafe_buffer_str(dst, ")\n");
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

void
tym_free_universe(struct TymUniverse * uni)
{
  if (uni->cardinality > 0) {
    for (int i = 0; i < uni->cardinality; i++) {
      tym_free_str(uni->element[i]);
    }
    free(uni->element);
  }
  free(uni);
}

struct TymStmt *
tym_mk_stmt_axiom(struct TymFmla * axiom)
{
  struct TymStmt * result = malloc(sizeof *result);
  *result = (struct TymStmt){.kind = TYM_STMT_AXIOM, .param.axiom = axiom};
  return result;
}

struct TymStmt *
tym_mk_stmt_pred(const TymStr * pred_name, struct TymTerms * params, struct TymFmla * body)
{
  struct TymStmt * result = malloc(sizeof *result);
  struct TymStmtConst * sub_result = malloc(sizeof *sub_result);

  *sub_result = (struct TymStmtConst)
      {.const_name = pred_name,
       .params = params,
       .body = body,
       .ty = TYM_CSTR_DUPLICATE(tym_bool_ty)};

  result->kind = TYM_STMT_CONST_DEF;
  result->param.const_def = sub_result;
  return result;
}

struct TymStmt *
tym_split_stmt_pred(struct TymStmt * stmt)
{
  struct TymFmla * def = stmt->param.const_def->body;
  stmt->param.const_def->body = NULL;

  uint8_t length = tym_len_TymTerms_cell(stmt->param.const_def->params);
  struct TymTerm ** predargs = malloc(sizeof *predargs * length);
  struct TymTerms * cursor = stmt->param.const_def->params;
  uint8_t i = 0;
  while (NULL != cursor) {
    predargs[i] = tym_copy_term(cursor->term);
    i++;
    cursor = cursor->next;
  }

  struct TymFmla * head =
    tym_mk_fmla_atom(TYM_STR_DUPLICATE(stmt->param.const_def->const_name), length, predargs);

  def = tym_mk_fmla_iff(def, head);

  cursor = stmt->param.const_def->params;
  struct TymTerms * params = NULL;
  while (NULL != cursor) {
    params = tym_mk_term_cell(cursor->term, params);
    cursor = cursor->next;
  }
  def = tym_mk_fmla_quants(FMLA_ALL, params, def);

  tym_shallow_free_terms(params);

  return tym_mk_stmt_axiom(def);
}

struct TymStmt *
tym_mk_stmt_const(const TymStr * const_name, struct TymUniverse * uni, const TymStr * ty)
{
  assert(NULL != const_name);
  assert(NULL != uni);
  assert(uni->cardinality > 0);

  struct TymStmt * result = malloc(sizeof *result);
  struct TymStmtConst * sub_result = malloc(sizeof *sub_result);

  *sub_result = (struct TymStmtConst)
    {.const_name = const_name,
     .params = NULL,
     .body = NULL,
     .ty = ty};

  result->kind = TYM_STMT_CONST_DEF;
  result->param.const_def = sub_result;
  return result;
}

struct TymStmt *
tym_mk_stmt_const_def(const TymStr * const_name, struct TymUniverse * uni)
{
  assert(NULL != const_name);
  assert(NULL != uni);
  assert(uni->cardinality > 0);

  struct TymFmlas * fmlas = NULL;

  for (int i = 0; i < uni->cardinality; i++) {
    if (i > 0) {
      // Currently terms must have disjoint strings, since these are freed
      // up independently (without checking if a shared string has already been
      // freed, say).
      const_name = TYM_STR_DUPLICATE(const_name);
    }
    struct TymTerm * arg1 = tym_mk_term(TYM_CONST, const_name);
    struct TymTerm * arg2 =
      tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(uni->element[i]));
    struct TymFmla * fmla = tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE(tym_eqK),
        2, arg1, arg2);
    fmlas = tym_mk_fmla_cell(fmla, fmlas);
  }

  return tym_mk_stmt_axiom(tym_mk_fmla_ors(fmlas));
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_stmt_str(const struct TymStmt * const stmt, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  switch (stmt->kind) {
  case TYM_STMT_AXIOM:
    res = tym_buf_strcpy(dst, "(assert");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

    res = tym_fmla_str(stmt->param.axiom, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    break;

  case TYM_STMT_CONST_DEF:
    // Check arity, and use define-fun or declare-const as appropriate.

    if (NULL == stmt->param.const_def->params &&
        0 == strcmp(tym_decode_str(stmt->param.const_def->ty), TYM_UNIVERSE_TY)) {
      // We're dealing with a nullary constant.
      res = tym_buf_strcpy(dst, "(declare-const");
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = tym_buf_strcpy(dst, tym_decode_str(stmt->param.const_def->const_name));
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = tym_buf_strcpy(dst, tym_decode_str(stmt->param.const_def->ty));
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    } else {
      bool is_only_declaration = (NULL == stmt->param.const_def->body);
      if (is_only_declaration) {
        res = tym_buf_strcpy(dst, "(declare-fun");
      } else {
        res = tym_buf_strcpy(dst, "(define-fun");
      }
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = tym_buf_strcpy(dst, tym_decode_str(stmt->param.const_def->const_name));
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      if (tym_have_space(dst, 1)) {
        tym_unsafe_buffer_char(dst, '(');
      } else {
        return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
      }

      const struct TymTerms * params_cursor = stmt->param.const_def->params;
      while (NULL != params_cursor) {
        if (tym_have_space(dst, 1)) {
          tym_unsafe_buffer_char(dst, '(');
        } else {
          return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
        }

        if (!is_only_declaration) {
          res = tym_term_str(params_cursor->term, dst);
          assert(tym_is_ok_TymBufferWriteResult(res));
          free(res);

          tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
        }

        res = tym_buf_strcpy(dst, TYM_UNIVERSE_TY);
        assert(tym_is_ok_TymBufferWriteResult(res));
        free(res);

        tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.

        params_cursor = params_cursor->next;
        if (NULL != params_cursor) {
          if (tym_have_space(dst, 1)) {
            tym_unsafe_buffer_char(dst, ' ');
          } else {
            return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
          }
        }
      }

      if (tym_have_space(dst, 2)) {
        tym_unsafe_buffer_char(dst, ')');
        tym_unsafe_buffer_char(dst, ' ');
      } else {
        return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
      }

      res = tym_buf_strcpy(dst, tym_decode_str(stmt->param.const_def->ty));
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      if (!is_only_declaration) {
        tym_safe_buffer_replace_last(dst, '\n'); // replace the trailing \0.
      } else {
        tym_safe_buffer_replace_last(dst, ' ');
      }

      if (tym_have_space(dst, 1)) {
        tym_unsafe_buffer_char(dst, ' ');
      } else {
        return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
      }

      if (!is_only_declaration) {
        res = tym_fmla_str(stmt->param.const_def->body, dst);
        assert(tym_is_ok_TymBufferWriteResult(res));
        free(res);
      }

      tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    }
    break;
  default:
    return tym_mkerrval_TymBufferWriteResult(NON_BUFF_ERROR);
  }

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_stmt(const struct TymStmt * stmt)
{
  switch (stmt->kind) {
  case TYM_STMT_AXIOM:
    tym_free_fmla(stmt->param.axiom);
    break;
  case TYM_STMT_CONST_DEF:
    tym_free_str(stmt->param.const_def->const_name);
    if (NULL != stmt->param.const_def->params) {
      tym_free_terms(stmt->param.const_def->params);
    }
    if (NULL != stmt->param.const_def->body) {
      tym_free_fmla(stmt->param.const_def->body);
    }
    tym_free_str(stmt->param.const_def->ty);
    free(stmt->param.const_def);
    break;
  default:
    assert(false);
    break;
  }
  free((void *)stmt);
}
#pragma GCC diagnostic pop

TYM_DEFINE_MUTABLE_LIST_MK(stmt, stmt, struct TymStmt, struct TymStmts)

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_stmts_str(const struct TymStmts * const stmts, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);
  const struct TymStmts * cursor = stmts;

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  while (NULL != cursor) {
    res = tym_stmt_str(cursor->stmt, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, '\n'); // replace the trailing \0.

    cursor = cursor->next;
  }

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_stmts(const struct TymStmts * stmts)
{
  assert(NULL != stmts->stmt);
  tym_free_stmt(stmts->stmt);
  if (NULL != stmts->next) {
    tym_free_stmts(stmts->next);
  }
  free((void *)stmts);
}
#pragma GCC diagnostic pop

struct TymModel *
tym_mk_model(struct TymUniverse * uni)
{
  struct TymModel * result = malloc(sizeof *result);
  result->universe = uni;
  result->stmts = NULL;
  return result;
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_model_str(const struct TymModel * const mdl, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_buf_strcpy(dst, "(declare-sort");
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  res = tym_buf_strcpy(dst, TYM_UNIVERSE_TY);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  if (tym_have_space(dst, 3)) {
    tym_unsafe_buffer_str(dst, "0)\n");
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }

  res = tym_stmts_str(mdl->stmts, dst);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_model(const struct TymModel * mdl)
{
  tym_free_universe(mdl->universe);
  if (NULL != mdl->stmts) {
    tym_free_stmts(mdl->stmts);
  }
  free((void *)mdl);
}
#pragma GCC diagnostic pop

void
tym_strengthen_model(struct TymModel * mdl, struct TymStmt * stmt)
{
  struct TymStmts * stmts = mdl->stmts;
  mdl->stmts = tym_mk_stmt_cell(stmt, stmts);
}

void
tym_test_statement(void)
{
  printf("***test_statement***\n");
  struct TymTerm * aT = tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("a"));
  struct TymTerm * bT = tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("b"));
  struct TymTerms * terms = tym_mk_term_cell(aT, NULL);
  terms = tym_mk_term_cell(bT, terms);

  struct TymModel * mdl = tym_mk_model(tym_mk_universe(terms));
  tym_free_terms(terms);

  struct TymStmt * s1S =
    tym_mk_stmt_axiom(tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE(tym_eqK), 2,
          tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("a")),
          tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("a"))));
  terms = tym_mk_term_cell(tym_mk_term(TYM_VAR, TYM_CSTR_DUPLICATE("X")), NULL);
  terms = tym_mk_term_cell(tym_mk_term(TYM_VAR, TYM_CSTR_DUPLICATE("Y")), terms);
  struct TymFmla * fmla =
    tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE(tym_eqK), 2,
        tym_mk_term(TYM_VAR, TYM_CSTR_DUPLICATE("X")),
        tym_mk_term(TYM_VAR, TYM_CSTR_DUPLICATE("Y")));
  struct TymStmt * s2S = tym_mk_stmt_pred(TYM_CSTR_DUPLICATE("some_predicate"),
      terms,
      tym_mk_fmla_not(fmla));
  struct TymStmt * s3AS = tym_mk_stmt_const(TYM_CSTR_DUPLICATE("x"),
      mdl->universe, TYM_CSTR_DUPLICATE(TYM_UNIVERSE_TY));
  struct TymStmt * s3BS = tym_mk_stmt_const_def(TYM_CSTR_DUPLICATE("x"),
      mdl->universe);

  tym_strengthen_model(mdl, s1S);
  tym_strengthen_model(mdl, s2S);
  tym_strengthen_model(mdl, s3AS);
  tym_strengthen_model(mdl, s3BS);

  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_model_str(mdl, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  TYM_DBG_BUFFER(outbuf, "test model")
  tym_free_buffer(outbuf);

  tym_free_model(mdl);
}

TYM_DEFINE_LIST_REV(stmt, stmts, tym_mk_stmt_cell, , struct TymStmts, )

struct TymTerm *
tym_new_const_in_stmt(const struct TymStmt * stmt)
{
  struct TymTerm * result = NULL;

  switch (stmt->kind) {
  case TYM_STMT_AXIOM:
    result = NULL;
    break;
  case TYM_STMT_CONST_DEF:
    result = tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(stmt->param.const_def->const_name));
    break;
  default:
    assert(false);
    break;
  }
  return result;
}

struct TymTerms *
tym_consts_in_stmt(const struct TymStmt * stmt)
{
  struct TymTerms * result = NULL;
  switch (stmt->kind) {
  case TYM_STMT_AXIOM:
    result = tym_consts_in_fmla(stmt->param.axiom, NULL, true);
    break;
  case TYM_STMT_CONST_DEF:
    if (NULL != stmt->param.const_def->body) {
      result = tym_consts_in_fmla(stmt->param.const_def->body, NULL, true);
    }
    break;
  default:
    assert(false);
    break;
  }
  return result;
}

void
tym_statementise_universe(struct TymModel * mdl)
{
  assert(NULL != mdl);

  if (0 == mdl->universe->cardinality) {
    return;
  }

  for (int i = 0; i < mdl->universe->cardinality; i++) {
    tym_strengthen_model(mdl,
        tym_mk_stmt_const(TYM_STR_DUPLICATE(mdl->universe->element[i]),
          mdl->universe, TYM_CSTR_DUPLICATE(TYM_UNIVERSE_TY)));
  }

  assert(0 < mdl->universe->cardinality);
  struct TymTerm ** args = malloc(sizeof *args * mdl->universe->cardinality);
  for (int i = 0; i < mdl->universe->cardinality; i++) {
    args[i] = tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(mdl->universe->element[i]));
  }
  const TymStr * copied = TYM_CSTR_DUPLICATE(tym_distinctK);
  struct TymFmla * distinctness_fmla =
    tym_mk_fmla_atom(copied, mdl->universe->cardinality, args);
  tym_strengthen_model(mdl, tym_mk_stmt_axiom(distinctness_fmla));

  struct TymSymGen * sg = tym_mk_sym_gen(TYM_CSTR_DUPLICATE("X"));
  const TymStr * varname = tym_mk_new_var(sg);

  struct TymFmlas * cardinality_fmlas = NULL;

  for (int i = 0; i < mdl->universe->cardinality; i++) {
    args = malloc(sizeof *args * 2);
    args[0] = tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(mdl->universe->element[i]));
    args[1] = tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(varname));
    struct TymFmla * fmla =
      tym_mk_fmla_atom(TYM_CSTR_DUPLICATE(tym_eqK), 2, args);
    cardinality_fmlas = tym_mk_fmla_cell(fmla, cardinality_fmlas);
  }

  struct TymFmla * cardinality_fmla = tym_mk_fmla_ors(cardinality_fmlas);
  cardinality_fmla = tym_mk_fmla_quant(FMLA_ALL, varname, cardinality_fmla);
  tym_strengthen_model(mdl, tym_mk_stmt_axiom(cardinality_fmla));

  tym_free_sym_gen(sg);
}
