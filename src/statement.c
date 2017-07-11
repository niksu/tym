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

char * bool_ty = "Bool";
char * distinctK = "distinct";
char * eqK = "=";

struct universe_t *
mk_universe(struct TymTerms * terms)
{
  struct universe_t * result = malloc(sizeof(struct universe_t));
  result->cardinality = 0;
  result->element = NULL;

  const struct TymTerms * cursor = terms;
  while (NULL != cursor) {
    result->cardinality++;
    assert(TYM_CONST == cursor->term->kind);
    cursor = cursor->next;
  }

  assert(result->cardinality > 0);
  result->element = malloc(sizeof(char *) * result->cardinality);

  cursor = terms;
  for (int i = 0; i < result->cardinality; i++) {
    result->element[i] = malloc(sizeof(char) * (strlen(cursor->term->identifier) + 1));
    strcpy(result->element[i], cursor->term->identifier);
    cursor = cursor->next;
  }

  return result;
}

struct TymBufferWriteResult *
universe_str(const struct universe_t * const uni, struct TymBufferInfo * dst)
{
  size_t initial_idx = dst->idx;

  struct TymBufferWriteResult * res = NULL;

  for (int i = 0; i < uni->cardinality; i++) {
    res = tym_buf_strcpy(dst, "(declare-const");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

    res = tym_buf_strcpy(dst, uni->element[i]);
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
    res = tym_buf_strcpy(dst, uni->element[i]);
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
free_universe(struct universe_t * uni)
{
  if (uni->cardinality > 0) {
    for (int i = 0; i < uni->cardinality; i++) {
      free(uni->element[i]);
    }
    free(uni->element);
  }
  free(uni);
}

const struct stmt_t *
mk_stmt_axiom(const struct TymFmla * axiom)
{
  struct stmt_t * result = malloc(sizeof(struct stmt_t));
  *result = (struct stmt_t){.kind = STMT_AXIOM, .param.axiom = axiom};
  return result;
}

const struct stmt_t *
mk_stmt_pred(char * pred_name, struct TymTerms * params, struct TymFmla * body)
{
  struct stmt_t * result = malloc(sizeof(struct stmt_t));
  struct stmt_const_t * sub_result = malloc(sizeof(struct stmt_const_t));

  *sub_result = (struct stmt_const_t)
      {.const_name = pred_name,
       .params = params,
       .body = body,
       .ty = bool_ty};

  result->kind = STMT_CONST_DEF;
  result->param.const_def = sub_result;
  return result;
}

struct stmt_t *
mk_stmt_const(char * const_name, struct universe_t * uni, char * ty)
{
  assert(NULL != const_name);
  assert(NULL != uni);
  assert(uni->cardinality > 0);

  struct stmt_t * result = malloc(sizeof(struct stmt_t));
  struct stmt_const_t * sub_result = malloc(sizeof(struct stmt_const_t));

  *sub_result = (struct stmt_const_t)
    {.const_name = const_name,
     .params = NULL,
     .body = NULL,
     .ty = ty};

  result->kind = STMT_CONST_DEF;
  result->param.const_def = sub_result;
  return result;
}

const struct stmt_t *
mk_stmt_const_def(char * const_name, struct universe_t * uni)
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
      const_name = strdup(const_name);
    }
    struct TymTerm * arg1 = tym_mk_term(TYM_CONST, const_name);
    struct TymTerm * arg2 = tym_mk_term(TYM_CONST, strdup(uni->element[i]));
    struct TymFmla * fmla = tym_mk_fmla_atom_varargs(strdup(eqK), 2, arg1, arg2);
    fmlas = tym_mk_fmla_cell(fmla, fmlas);
  }

  return mk_stmt_axiom(tym_mk_fmla_ors(fmlas));
}

struct TymBufferWriteResult *
stmt_str(const struct stmt_t * const stmt, struct TymBufferInfo * dst)
{
  size_t initial_idx = dst->idx;

  struct TymBufferWriteResult * res = NULL;

  switch (stmt->kind) {
  case STMT_AXIOM:
    res = tym_buf_strcpy(dst, "(assert");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

    res = tym_fmla_str(stmt->param.axiom, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    break;

  case STMT_CONST_DEF:
    // Check arity, and use define-fun or declare-const as appropriate.

    if (NULL == stmt->param.const_def->params && stmt->param.const_def->ty == TYM_UNIVERSE_TY) {
      // We're dealing with a nullary constant.
      res = tym_buf_strcpy(dst, "(declare-const");
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = tym_buf_strcpy(dst, stmt->param.const_def->const_name);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = tym_buf_strcpy(dst, stmt->param.const_def->ty);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    } else {
      res = tym_buf_strcpy(dst, "(define-fun");
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = tym_buf_strcpy(dst, stmt->param.const_def->const_name);
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

      res = tym_buf_strcpy(dst, stmt->param.const_def->ty);
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
free_stmt(const struct stmt_t * stmt)
{
  switch (stmt->kind) {
  case STMT_AXIOM:
    tym_free_fmla(stmt->param.axiom);
    break;
  case STMT_CONST_DEF:
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

TYM_DEFINE_LIST_MK(stmt, stmt, struct stmt_t, struct stmts_t, const)

struct TymBufferWriteResult *
stmts_str(const struct stmts_t * const stmts, struct TymBufferInfo * dst)
{
  size_t initial_idx = dst->idx;
  const struct stmts_t * cursor = stmts;

  struct TymBufferWriteResult * res = NULL;

  while (NULL != cursor) {
    res = stmt_str(cursor->stmt, dst);
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
free_stmts(const struct stmts_t * stmts)
{
  assert(NULL != stmts->stmt);
  free_stmt(stmts->stmt);
  if (NULL != stmts->next) {
    free_stmts(stmts->next);
  }
  free((void *)stmts);
}
#pragma GCC diagnostic pop

struct model_t *
mk_model(struct universe_t * uni)
{
  struct model_t * result = malloc(sizeof(struct model_t));
  result->universe = uni;
  result->stmts = NULL;
  return result;
}

struct TymBufferWriteResult *
model_str(const struct model_t * const mdl, struct TymBufferInfo * dst)
{
  size_t initial_idx = dst->idx;

  struct TymBufferWriteResult * res = tym_buf_strcpy(dst, "(declare-sort");
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

  res = stmts_str(mdl->stmts, dst);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  return tym_mkval_TymBufferWriteResult(dst->idx - initial_idx);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_model(const struct model_t * mdl)
{
  free_universe(mdl->universe);
  if (NULL != mdl->stmts) {
    free_stmts(mdl->stmts);
  }
  free((void *)mdl);
}
#pragma GCC diagnostic pop

void
strengthen_model(struct model_t * mdl, const struct stmt_t * stmt)
{
  const struct stmts_t * stmts = mdl->stmts;
  mdl->stmts = tym_mk_stmt_cell(stmt, stmts);
}

void
tym_test_statement(void)
{
  printf("***test_statement***\n");
  struct TymTerm * aT = tym_mk_term(TYM_CONST, strdup("a"));
  struct TymTerm * bT = tym_mk_term(TYM_CONST, strdup("b"));
  struct TymTerms * terms = tym_mk_term_cell(aT, NULL);
  terms = tym_mk_term_cell(bT, terms);

  struct model_t * mdl = mk_model(mk_universe(terms));
  tym_free_terms(terms);

  const struct stmt_t * s1S =
    mk_stmt_axiom(tym_mk_fmla_atom_varargs(strdup(eqK), 2, tym_mk_const("a"), tym_mk_const("a")));
  terms = tym_mk_term_cell(tym_mk_term(TYM_VAR, strdup("X")), NULL);
  terms = tym_mk_term_cell(tym_mk_term(TYM_VAR, strdup("Y")), terms);
  struct TymFmla * fmla =
    tym_mk_fmla_atom_varargs(strdup(eqK), 2, tym_mk_var("X"), tym_mk_var("Y"));
  const struct stmt_t * s2S = mk_stmt_pred(strdup("some_predicate"), terms,
      tym_mk_fmla_not(fmla));
  struct stmt_t * s3AS = mk_stmt_const(strdup("x"), mdl->universe, TYM_UNIVERSE_TY);
  const struct stmt_t * s3BS = mk_stmt_const_def(strdup("x"), mdl->universe);

  strengthen_model(mdl, s1S);
  strengthen_model(mdl, s2S);
  strengthen_model(mdl, s3AS);
  strengthen_model(mdl, s3BS);

  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TymBufferWriteResult * res = model_str(mdl, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  printf("test model (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);
  tym_free_buffer(outbuf);

  free_model(mdl);
}

TYM_DEFINE_LIST_REV(stmts, tym_mk_stmt_cell, const, struct stmts_t, const)

struct TymTerm *
new_const_in_stmt(const struct stmt_t * stmt)
{
  struct TymTerm * result = NULL;
  switch (stmt->kind) {
  case STMT_AXIOM:
    result = NULL;
    break;
  case STMT_CONST_DEF:
    result = tym_mk_term(TYM_CONST, strdup(stmt->param.const_def->const_name));
    break;
  default:
    assert(false);
    break;
  }
  return result;
}

struct TymTerms *
consts_in_stmt(const struct stmt_t * stmt)
{
  struct TymTerms * result = NULL;
  switch (stmt->kind) {
  case STMT_AXIOM:
    result = tym_consts_in_fmla(stmt->param.axiom, NULL);
    break;
  case STMT_CONST_DEF:
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
statementise_universe(struct model_t * mdl)
{
  assert(NULL != mdl);

  if (0 == mdl->universe->cardinality) {
    return;
  }

  for (int i = 0; i < mdl->universe->cardinality; i++) {
    strengthen_model(mdl,
        mk_stmt_const(strdup(mdl->universe->element[i]), mdl->universe, TYM_UNIVERSE_TY));
  }

  assert(0 < mdl->universe->cardinality);
  struct TymTerm ** args = malloc(sizeof(struct Term *) * mdl->universe->cardinality);
  for (int i = 0; i < mdl->universe->cardinality; i++) {
    args[i] = tym_mk_term(TYM_CONST, strdup(mdl->universe->element[i]));
  }
  const struct TymFmla * distinctness_fmla =
    tym_mk_fmla_atom(strdup(distinctK), mdl->universe->cardinality, args);
  strengthen_model(mdl, mk_stmt_axiom(distinctness_fmla));
}
