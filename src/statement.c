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

  assert(result->cardinality > 0);
  result->element = malloc(sizeof *result->element * result->cardinality);

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
  size_t initial_idx = dst->idx;

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
    return tym_mkval_TymBufferWriteResult(dst->idx - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

void
tym_free_universe(struct TymUniverse * uni)
{
  if (uni->cardinality > 0) {
    for (int i = 0; i < uni->cardinality; i++) {
      free(uni->element[i]);
    }
    free(uni->element);
  }
  free(uni);
}

const struct TymStmt *
tym_mk_stmt_axiom(const struct TymFmla * axiom)
{
  struct TymStmt * result = malloc(sizeof *result);
  *result = (struct TymStmt){.kind = TYM_STMT_AXIOM, .param.axiom = axiom};
  return result;
}

const struct TymStmt *
tym_mk_stmt_pred(TymStr * pred_name, struct TymTerms * params, struct TymFmla * body)
{
  struct TymStmt * result = malloc(sizeof *result);
  struct TymStmtConst * sub_result = malloc(sizeof *sub_result);

  *sub_result = (struct TymStmtConst)
      {.const_name = pred_name,
       .params = params,
       .body = body,
       .ty = tym_encode_str(tym_bool_ty)/*FIXME hack*/};

  result->kind = TYM_STMT_CONST_DEF;
  result->param.const_def = sub_result;
  return result;
}

struct TymStmt *
tym_mk_stmt_const(TymStr * const_name, struct TymUniverse * uni, TymStr * ty)
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

const struct TymStmt *
tym_mk_stmt_const_def(TymStr * const_name, struct TymUniverse * uni)
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
      TymStr * copied = TYM_STR_DUPLICATE(const_name);
      const_name = copied;
    }
    struct TymTerm * arg1 = tym_mk_term(TYM_CONST, const_name);
    TymStr * copied = TYM_STR_DUPLICATE(uni->element[i]);
    struct TymTerm * arg2 = tym_mk_term(TYM_CONST, copied);
    copied = tym_encode_str(strdup(tym_eqK)); // FIXME hack
    struct TymFmla * fmla = tym_mk_fmla_atom_varargs(copied, 2, arg1, arg2);
    fmlas = tym_mk_fmla_cell(fmla, fmlas);
  }

  return tym_mk_stmt_axiom(tym_mk_fmla_ors(fmlas));
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_stmt_str(const struct TymStmt * const stmt, struct TymBufferInfo * dst)
{
  size_t initial_idx = dst->idx;

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

    if (NULL == stmt->param.const_def->params && tym_decode_str(stmt->param.const_def->ty) == TYM_UNIVERSE_TY/*FIXME hack*/) {
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
      res = tym_buf_strcpy(dst, "(define-fun");
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

        res = tym_term_to_str(params_cursor->term, dst);
        assert(tym_is_ok_TymBufferWriteResult(res));
        free(res);

        tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

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

      tym_safe_buffer_replace_last(dst, '\n'); // replace the trailing \0.

      if (tym_have_space(dst, 1)) {
        tym_unsafe_buffer_char(dst, ' ');
      } else {
        return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
      }

      res = tym_fmla_str(stmt->param.const_def->body, dst);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    }
    break;
  default:
    return tym_mkerrval_TymBufferWriteResult(NON_BUFF_ERROR);
  }

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(dst->idx - initial_idx);
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
    free((void *)stmt->param.const_def->const_name);
    if (NULL != stmt->param.const_def->params) {
      tym_free_terms(stmt->param.const_def->params);
    }
    if (NULL != stmt->param.const_def->body) {
      tym_free_fmla(stmt->param.const_def->body);
    }
    free(stmt->param.const_def);
    break;
  default:
    assert(false);
    break;
  }
  free((void *)stmt);
}
#pragma GCC diagnostic pop

TYM_DEFINE_LIST_MK(stmt, stmt, struct TymStmt, struct TymStmts, const)

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_stmts_str(const struct TymStmts * const stmts, struct TymBufferInfo * dst)
{
  size_t initial_idx = dst->idx;
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
    return tym_mkval_TymBufferWriteResult(dst->idx - initial_idx);
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
  size_t initial_idx = dst->idx;

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

  return tym_mkval_TymBufferWriteResult(dst->idx - initial_idx);
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
tym_strengthen_model(struct TymModel * mdl, const struct TymStmt * stmt)
{
  const struct TymStmts * stmts = mdl->stmts;
  mdl->stmts = tym_mk_stmt_cell(stmt, stmts);
}

void
tym_test_statement(void)
{
  printf("***test_statement***\n");
  struct TymTerm * aT = tym_mk_term(TYM_CONST, tym_encode_str(strdup("a")));
  struct TymTerm * bT = tym_mk_term(TYM_CONST, tym_encode_str(strdup("b")));
  struct TymTerms * terms = tym_mk_term_cell(aT, NULL);
  terms = tym_mk_term_cell(bT, terms);

  struct TymModel * mdl = tym_mk_model(tym_mk_universe(terms));
  tym_free_terms(terms);

  const struct TymStmt * s1S =
    tym_mk_stmt_axiom(tym_mk_fmla_atom_varargs(tym_encode_str(strdup(tym_eqK)), 2, tym_mk_const(tym_encode_str("a")), tym_mk_const(tym_encode_str("a"))));
  terms = tym_mk_term_cell(tym_mk_term(TYM_VAR, tym_encode_str(strdup("X"))), NULL);
  terms = tym_mk_term_cell(tym_mk_term(TYM_VAR, tym_encode_str(strdup("Y"))), terms);
  struct TymFmla * fmla =
    tym_mk_fmla_atom_varargs(tym_encode_str(strdup(tym_eqK)), 2, tym_mk_var(tym_encode_str("X")), tym_mk_var(tym_encode_str("Y")));
  const struct TymStmt * s2S = tym_mk_stmt_pred(tym_encode_str(strdup("some_predicate")), terms,
      tym_mk_fmla_not(fmla));
  struct TymStmt * s3AS = tym_mk_stmt_const(tym_encode_str(strdup("x")), mdl->universe, tym_encode_str(TYM_UNIVERSE_TY));
  const struct TymStmt * s3BS = tym_mk_stmt_const_def(tym_encode_str(strdup("x")), mdl->universe);

  tym_strengthen_model(mdl, s1S);
  tym_strengthen_model(mdl, s2S);
  tym_strengthen_model(mdl, s3AS);
  tym_strengthen_model(mdl, s3BS);

  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_model_str(mdl, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  printf("test model (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);
  tym_free_buffer(outbuf);

  tym_free_model(mdl);
}

TYM_DEFINE_LIST_REV(stmts, tym_mk_stmt_cell, const, struct TymStmts, const)

struct TymTerm *
tym_new_const_in_stmt(const struct TymStmt * stmt)
{
  struct TymTerm * result = NULL;
  TymStr * copied;

  switch (stmt->kind) {
  case TYM_STMT_AXIOM:
    result = NULL;
    break;
  case TYM_STMT_CONST_DEF:
    copied = TYM_STR_DUPLICATE(stmt->param.const_def->const_name);
    result = tym_mk_term(TYM_CONST, copied);
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
    result = tym_consts_in_fmla(stmt->param.axiom, NULL);
    break;
  case TYM_STMT_CONST_DEF:
    if (NULL != stmt->param.const_def->body) {
      result = tym_consts_in_fmla(stmt->param.const_def->body, NULL);
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
    TymStr * copied = TYM_STR_DUPLICATE(mdl->universe->element[i]);
    tym_strengthen_model(mdl,
        tym_mk_stmt_const(copied, mdl->universe, tym_encode_str(TYM_UNIVERSE_TY)));
  }

  assert(0 < mdl->universe->cardinality);
  struct TymTerm ** args = malloc(sizeof *args * mdl->universe->cardinality);
  for (int i = 0; i < mdl->universe->cardinality; i++) {
    TymStr * copied = TYM_STR_DUPLICATE(mdl->universe->element[i]);
    args[i] = tym_mk_term(TYM_CONST, copied);
  }
  TymStr * copied = tym_encode_str(strdup(tym_distinctK)); // FIXME hack
  const struct TymFmla * distinctness_fmla =
    tym_mk_fmla_atom(copied, mdl->universe->cardinality, args);
  tym_strengthen_model(mdl, tym_mk_stmt_axiom(distinctness_fmla));
}
