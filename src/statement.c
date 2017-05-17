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

struct universe_t *
mk_universe(struct terms_t * terms)
{
  struct universe_t * result = malloc(sizeof(struct universe_t));
  result->cardinality = 0;
  result->element = NULL;

  const struct terms_t * cursor = terms;
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

size_t
universe_str(struct universe_t * uni, size_t * remaining, char * buf)
{
  size_t l = 0;

  for (int i = 0; i < uni->cardinality; i++) {
    sprintf(&(buf[l]), "(declare-const %s %s)\n",
        uni->element[i],
        universe_ty);
    *remaining -= strlen(&(buf[l]));
    l += strlen(&(buf[l]));
  }

  sprintf(&(buf[l]), "(assert (distinct ");
  *remaining -= strlen(&(buf[l]));
  l += strlen(&(buf[l]));
  for (int i = 0; i < uni->cardinality; i++) {
    sprintf(&(buf[l]), "%s", uni->element[i]);
    *remaining -= strlen(&(buf[l]));
    l += strlen(&(buf[l]));

    if (i < uni->cardinality - 1) {
      buf[(*remaining)--, l++] = ' ';
    }
  }

  buf[(*remaining)--, l++] = ')';
  buf[(*remaining)--, l++] = ')';
  buf[(*remaining)--, l++] = '\n';

  buf[l] = '\0';
  return l;
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
mk_stmt_pred(const char * const pred_name, struct terms_t * params, const struct fmla_t * body)
{
  struct stmt_t * result = malloc(sizeof(struct stmt_t));
  struct stmt_const_t * sub_result = malloc(sizeof(struct stmt_const_t));
// FIXME copy
//  struct terms_t * params_copy =
//  struct fmla_t * body_copy =
  char * const_name_copy = malloc(sizeof(char) * (strlen(pred_name) + 1));
  strcpy(const_name_copy, pred_name);
  sub_result->const_name = const_name_copy;

  *sub_result = (struct stmt_const_t)
      {.const_name = const_name_copy,
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

  char * const_name_copy = malloc(sizeof(char) * (strlen(const_name) + 1));
  strcpy(const_name_copy, const_name);
  *sub_result = (struct stmt_const_t)
    {.const_name = const_name_copy,
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

  const struct fmlas_t * fmlas = NULL;

  for (int i = 0; i < uni->cardinality; i++) {
    const struct fmla_t * fmla = mk_fmla_atom_varargs("=", 2,
      mk_term(CONST, const_name),
      mk_term(CONST, uni->element[i]));
    fmlas = mk_fmla_cell(fmla, fmlas);
  }

  return mk_stmt_axiom(mk_fmla_ors(fmlas));
}

size_t
stmt_str(const struct stmt_t * stmt, size_t * remaining, char * buf)
{
  size_t l = 0;
  switch (stmt->kind) {
  case STMT_AXIOM:
    sprintf(&(buf[l]), "(assert ");
    *remaining -= strlen(&(buf[l]));
    l += strlen(&(buf[l]));

    l += fmla_str(stmt->param.axiom, remaining, buf + l);

    buf[(*remaining)--, l++] = ')';
    break;

  case STMT_CONST_DEF:
    // Check arity, and use define-fun or declare-const as appropriate.

    if (NULL == stmt->param.const_def->params && stmt->param.const_def->ty == universe_ty) {
      // We're dealing with a nullary constant.
      sprintf(&(buf[l]), "(declare-const %s %s)",
          stmt->param.const_def->const_name,
          stmt->param.const_def->ty);
      *remaining -= strlen(&(buf[l]));
      l += strlen(&(buf[l]));
    } else {
      sprintf(&(buf[l]), "(define-fun %s (",
          stmt->param.const_def->const_name);
      *remaining -= strlen(&(buf[l]));
      l += strlen(&(buf[l]));

      const struct terms_t * params_cursor = stmt->param.const_def->params;
      while (NULL != params_cursor) {
        buf[(*remaining)--, l++] = '(';

        l += term_to_str(params_cursor->term, remaining, buf + l);

        sprintf(&(buf[l]), " %s)", universe_ty);
        *remaining -= strlen(&(buf[l]));
        l += strlen(&(buf[l]));

        params_cursor = params_cursor->next;
        if (NULL != params_cursor) {
          buf[(*remaining)--, l++] = ' ';
        }
      }

      sprintf(&(buf[l]), ") %s\n  ", stmt->param.const_def->ty);
      *remaining -= strlen(&(buf[l]));
      l += strlen(&(buf[l]));

      l += fmla_str(stmt->param.const_def->body, remaining, buf + l);

      buf[(*remaining)--, l++] = ')';
    }
    break;

  default:
    // FIXME complain
    return 0;
  }

  return l;
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
      free_terms(stmt->param.const_def->params);
    }
    if (NULL != stmt->param.const_def->body) {
      free_fmla(stmt->param.const_def->body);
    }
    free(stmt->param.const_def);
    break;
  default:
    // FIXME complain
    break;
  }
  free((void *)stmt);
}
#pragma GCC diagnostic pop

DEFINE_LIST_MK(stmt, stmt, struct stmt_t, struct stmts_t, const)

size_t
stmts_str(const struct stmts_t * stmts, size_t * remaining, char * buf)
{
  size_t l = 0;
  const struct stmts_t * cursor = stmts;
  while (NULL != cursor) {
    l += stmt_str(cursor->stmt, remaining, buf + l);
    buf[(*remaining)--, l++] = '\n';
    cursor = cursor->next;
  }
  return l;
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

size_t
model_str(struct model_t * mdl, size_t * remaining, char * buf)
{
  size_t l = 0;

  sprintf(&(buf[l]), "(declare-sort %s 0)\n", universe_ty);
  *remaining -= strlen(&(buf[l]));
  l += strlen(&(buf[l]));

  l += stmts_str(mdl->stmts, remaining, buf + l);
  buf[l] = '\0';
  return l;
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
  mdl->stmts = mk_stmt_cell(stmt, mdl->stmts);
}

void
test_statement(void)
{
  printf("***test_statement***\n");
  struct term_t * aT = mk_term(CONST, "a");
  struct term_t * bT = mk_term(CONST, "b");
  struct terms_t * terms = mk_term_cell(aT, NULL);
  terms = mk_term_cell(bT, terms);

  struct model_t * mdl = mk_model(mk_universe(terms));

  const struct stmt_t * s1S = mk_stmt_axiom(mk_fmla_atom_varargs("=", 2, mk_const("a"), mk_const("a")));
  char * vX = malloc(sizeof(char) * 2);
  strcpy(vX, "X");
  char * vY = malloc(sizeof(char) * 2);
  strcpy(vY, "Y");
  terms = mk_term_cell(mk_term(VAR, vX), NULL);
  terms = mk_term_cell(mk_term(VAR, vY), terms);
  const struct stmt_t * s2S = mk_stmt_pred("some_predicate", terms, mk_fmla_not(mk_fmla_atom_varargs("=", 2, mk_var("X"), mk_var("Y"))));
  struct stmt_t * s3AS = mk_stmt_const("x", mdl->universe, universe_ty);
  const struct stmt_t * s3BS = mk_stmt_const_def("x", mdl->universe);

  strengthen_model(mdl, s1S);
  strengthen_model(mdl, s2S);
  strengthen_model(mdl, s3AS);
  strengthen_model(mdl, s3BS);

  size_t remaining_buf_size = BUF_SIZE;
  char * buf = malloc(remaining_buf_size);
  size_t l = model_str(mdl, &remaining_buf_size, buf);
  printf("test model (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  free_model(mdl);
  free(buf);
}

DEFINE_LIST_REV(stmts, mk_stmt_cell, const, struct stmts_t, const)

struct term_t *
new_const_in_stmt(const struct stmt_t * stmt)
{
  struct term_t * result = NULL;
  switch (stmt->kind) {
  case STMT_AXIOM:
    result = NULL;
    break;
  case STMT_CONST_DEF:
    result = mk_term(CONST, stmt->param.const_def->const_name);
    break;
  default:
    // FIXME complain -- impossible result
    break;
  }
  return result;
}

struct terms_t *
consts_in_stmt(const struct stmt_t * stmt)
{
  struct terms_t * result = NULL;
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
    // FIXME complain -- impossible result
    break;
  }
  return result;
}

void
statementise_universe(struct model_t * mdl)
{
  for (int i = 0; i < mdl->universe->cardinality; i++) {
    strengthen_model(mdl, mk_stmt_const(mdl->universe->element[i], mdl->universe, universe_ty));
  }

  struct term_t ** args = malloc(sizeof(struct term_t *) * mdl->universe->cardinality);
  for (int i = 0; i < mdl->universe->cardinality; i++) {
    args[i] = mk_term(CONST, mdl->universe->element[i]);
  }
  const struct fmla_t * distinctness_fmla =
    mk_fmla_atom(distinct_pred, mdl->universe->cardinality, args);
  strengthen_model(mdl, mk_stmt_axiom(distinctness_fmla));
}
