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

const char * const bool_ty = "Bool";
const char * const distinctK = "distinct";
const char * const eqK = "=";

struct universe_t *
mk_universe(struct TymTerms * terms)
{
  struct universe_t * result = malloc(sizeof(struct universe_t));
  result->cardinality = 0;
  result->element = NULL;

  const struct TymTerms * cursor = terms;
  while (NULL != cursor) {
    result->cardinality++;
    assert(CONST == cursor->term->kind);
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

struct buffer_write_result *
universe_str(const struct universe_t * const uni, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = NULL;

  for (int i = 0; i < uni->cardinality; i++) {
    res = buf_strcpy(dst, "(declare-const");
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

    res = buf_strcpy(dst, uni->element[i]);
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

    res = buf_strcpy(dst, universe_ty);
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, ')'); // replace the trailing \0.

    if (have_space(dst, 1)) {
      unsafe_buffer_char(dst, '\n');
    } else {
      return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
    }
  }

  res = buf_strcpy(dst, "(assert (distinct");
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  for (int i = 0; i < uni->cardinality; i++) {
    res = buf_strcpy(dst, uni->element[i]);
    assert(is_ok_buffer_write_result(res));
    free(res);

    if (i < uni->cardinality - 1) {
      safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    } else {
      safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    }
  }

  if (have_space(dst, 3)) {
    unsafe_buffer_str(dst, ")\n");
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
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
mk_stmt_axiom(const struct fmla_t * axiom)
{
  struct stmt_t * result = malloc(sizeof(struct stmt_t));
  *result = (struct stmt_t){.kind = STMT_AXIOM, .param.axiom = axiom};
  return result;
}

const struct stmt_t *
mk_stmt_pred(char * pred_name, struct TymTerms * params, const struct fmla_t * body)
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
mk_stmt_const(char * const_name, struct universe_t * uni, const char * const ty)
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

  struct fmlas_t * fmlas = NULL;

  for (int i = 0; i < uni->cardinality; i++) {
    if (i > 0) {
      // Currently terms must have disjoint strings, since these are freed
      // up independently (without checking if a shared string has already been
      // freed, say).
      const_name = strdup(const_name);
    }
    struct TymTerm * arg1 = tym_mk_term(CONST, const_name);
    struct TymTerm * arg2 = tym_mk_term(CONST, strdup(uni->element[i]));
    struct fmla_t * fmla = mk_fmla_atom_varargs(strdup(eqK), 2, arg1, arg2);
    fmlas = tym_mk_fmla_cell(fmla, fmlas);
  }

  return mk_stmt_axiom(mk_fmla_ors(fmlas));
}

struct buffer_write_result *
stmt_str(const struct stmt_t * const stmt, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = NULL;

  switch (stmt->kind) {
  case STMT_AXIOM:
    res = buf_strcpy(dst, "(assert");
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

    res = fmla_str(stmt->param.axiom, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    break;

  case STMT_CONST_DEF:
    // Check arity, and use define-fun or declare-const as appropriate.

    if (NULL == stmt->param.const_def->params && stmt->param.const_def->ty == universe_ty) {
      // We're dealing with a nullary constant.
      res = buf_strcpy(dst, "(declare-const");
      assert(is_ok_buffer_write_result(res));
      free(res);

      safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = buf_strcpy(dst, stmt->param.const_def->const_name);
      assert(is_ok_buffer_write_result(res));
      free(res);

      safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = buf_strcpy(dst, stmt->param.const_def->ty);
      assert(is_ok_buffer_write_result(res));
      free(res);

      safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    } else {
      res = buf_strcpy(dst, "(define-fun");
      assert(is_ok_buffer_write_result(res));
      free(res);

      safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      res = buf_strcpy(dst, stmt->param.const_def->const_name);
      assert(is_ok_buffer_write_result(res));
      free(res);

      safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

      if (have_space(dst, 1)) {
        unsafe_buffer_char(dst, '(');
      } else {
        return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
      }

      const struct TymTerms * params_cursor = stmt->param.const_def->params;
      while (NULL != params_cursor) {
        if (have_space(dst, 1)) {
          unsafe_buffer_char(dst, '(');
        } else {
          return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
        }

        res = tym_term_to_str(params_cursor->term, dst);
        assert(is_ok_buffer_write_result(res));
        free(res);

        safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

        res = buf_strcpy(dst, universe_ty);
        assert(is_ok_buffer_write_result(res));
        free(res);

        safe_buffer_replace_last(dst, ')'); // replace the trailing \0.

        params_cursor = params_cursor->next;
        if (NULL != params_cursor) {
          if (have_space(dst, 1)) {
            unsafe_buffer_char(dst, ' ');
          } else {
            return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
          }
        }
      }

      if (have_space(dst, 2)) {
        unsafe_buffer_char(dst, ')');
        unsafe_buffer_char(dst, ' ');
      } else {
        return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
      }

      res = buf_strcpy(dst, stmt->param.const_def->ty);
      assert(is_ok_buffer_write_result(res));
      free(res);

      safe_buffer_replace_last(dst, '\n'); // replace the trailing \0.

      if (have_space(dst, 1)) {
        unsafe_buffer_char(dst, ' ');
      } else {
        return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
      }

      res = fmla_str(stmt->param.const_def->body, dst);
      assert(is_ok_buffer_write_result(res));
      free(res);

      safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
    }
    break;
  default:
    return mkerrval_buffer_write_result(NON_BUFF_ERROR);
  }

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_stmt(const struct stmt_t * stmt)
{
  switch (stmt->kind) {
  case STMT_AXIOM:
    free_fmla(stmt->param.axiom);
    break;
  case STMT_CONST_DEF:
    free((void *)stmt->param.const_def->const_name);
    if (NULL != stmt->param.const_def->params) {
      tym_free_terms(stmt->param.const_def->params);
    }
    if (NULL != stmt->param.const_def->body) {
      free_fmla(stmt->param.const_def->body);
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

struct buffer_write_result *
stmts_str(const struct stmts_t * const stmts, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;
  const struct stmts_t * cursor = stmts;

  struct buffer_write_result * res = NULL;

  while (NULL != cursor) {
    res = stmt_str(cursor->stmt, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, '\n'); // replace the trailing \0.

    cursor = cursor->next;
  }

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
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

struct buffer_write_result *
model_str(const struct model_t * const mdl, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, "(declare-sort");
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  res = buf_strcpy(dst, universe_ty);
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  if (have_space(dst, 3)) {
    unsafe_buffer_str(dst, "0)\n");
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }

  res = stmts_str(mdl->stmts, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  return mkval_buffer_write_result(dst->idx - initial_idx);
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
  struct TymTerm * aT = tym_mk_term(CONST, strdup("a"));
  struct TymTerm * bT = tym_mk_term(CONST, strdup("b"));
  struct TymTerms * terms = tym_mk_term_cell(aT, NULL);
  terms = tym_mk_term_cell(bT, terms);

  struct model_t * mdl = mk_model(mk_universe(terms));
  tym_free_terms(terms);

  const struct stmt_t * s1S =
    mk_stmt_axiom(mk_fmla_atom_varargs(strdup(eqK), 2, tym_mk_const("a"), tym_mk_const("a")));
  terms = tym_mk_term_cell(tym_mk_term(VAR, strdup("X")), NULL);
  terms = tym_mk_term_cell(tym_mk_term(VAR, strdup("Y")), terms);
  struct fmla_t * fmla =
    mk_fmla_atom_varargs(strdup(eqK), 2, tym_mk_var("X"), tym_mk_var("Y"));
  const struct stmt_t * s2S = mk_stmt_pred(strdup("some_predicate"), terms,
      mk_fmla_not(fmla));
  struct stmt_t * s3AS = mk_stmt_const(strdup("x"), mdl->universe, universe_ty);
  const struct stmt_t * s3BS = mk_stmt_const_def(strdup("x"), mdl->universe);

  strengthen_model(mdl, s1S);
  strengthen_model(mdl, s2S);
  strengthen_model(mdl, s3AS);
  strengthen_model(mdl, s3BS);

  struct buffer_info * outbuf = mk_buffer(TYM_BUF_SIZE);
  struct buffer_write_result * res = model_str(mdl, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test model (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);
  free_buffer(outbuf);

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
    result = tym_mk_term(CONST, strdup(stmt->param.const_def->const_name));
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
    result = consts_in_fmla(stmt->param.axiom, NULL);
    break;
  case STMT_CONST_DEF:
    if (NULL != stmt->param.const_def->body) {
      result = consts_in_fmla(stmt->param.const_def->body, NULL);
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
        mk_stmt_const(strdup(mdl->universe->element[i]), mdl->universe, universe_ty));
  }

  assert(0 < mdl->universe->cardinality);
  struct TymTerm ** args = malloc(sizeof(struct Term *) * mdl->universe->cardinality);
  for (int i = 0; i < mdl->universe->cardinality; i++) {
    args[i] = tym_mk_term(CONST, strdup(mdl->universe->element[i]));
  }
  const struct fmla_t * distinctness_fmla =
    mk_fmla_atom(strdup(distinctK), mdl->universe->cardinality, args);
  strengthen_model(mdl, mk_stmt_axiom(distinctness_fmla));
}
