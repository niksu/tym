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
#include "statement.h"
#include "tym.h"

struct universe_t *
mk_universe(struct terms_t * terms)
{
  struct universe_t * result = malloc(sizeof(struct universe_t));
  result->cardinality = 0;
  result->element = NULL;

  struct terms_t * cursor = terms;
  while (NULL != cursor) {
    result->cardinality++;
    assert(CONST == cursor->term->kind);
    cursor = cursor->next;
  }

  assert(result->cardinality > 0);
  result->element = malloc(sizeof(char *) * result->cardinality);

  cursor = terms;
  for (int i = 0; i < result->cardinality; i++) {
    result->element[i] = malloc(sizeof(char) * strlen(cursor->term->identifier));
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

struct stmt_t *
mk_stmt_axiom(struct fmla_t * axiom)
{
  struct stmt_t * result = malloc(sizeof(struct stmt_t));
  result->kind = STMT_AXIOM;
  result->param.axiom = axiom;
  return result;
}

struct stmt_t *
mk_stmt_pred(const char * const pred_name, struct terms_t * params, struct fmla_t * body)
{
  struct stmt_t * result = malloc(sizeof(struct stmt_t));
  struct stmt_const_t * sub_result = malloc(sizeof(struct stmt_const_t));
// FIXME copy
//  struct terms_t * params_copy =
//  struct fmla_t * body_copy =
  char * const_name_copy = malloc(sizeof(char) * strlen(pred_name));
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

  char * const_name_copy = malloc(sizeof(char) * strlen(const_name));
  strcpy(const_name_copy, const_name);
  *sub_result = (struct stmt_const_t)
    {.const_name = const_name_copy,
     .params = NULL,
     .body = NULL,
     .ty = ty};

  struct fmlas_t * fmlas = NULL;

  for (int i = 0; i < uni->cardinality; i++) {
    struct fmla_t * fmla = mk_fmla_atom_varargs("=", 2,
      mk_term(CONST, const_name),
      mk_term(CONST, uni->element[i]));
    fmlas = mk_fmla_cell(fmla, fmlas);
  }

  sub_result->body = mk_fmla_ors(fmlas);
  //free_fmlas(fmlas); FIXME include this to free memory used in intermediate computation.

  result->kind = STMT_CONST_DEF;
  result->param.const_def = sub_result;
  return result;
}

size_t
stmt_str(struct stmt_t * stmt, size_t * remaining, char * buf)
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
      sprintf(&(buf[l]), "(declare-const %s %s)\n",
          stmt->param.const_def->const_name,
          stmt->param.const_def->ty);
      *remaining -= strlen(&(buf[l]));
      l += strlen(&(buf[l]));

      sprintf(&(buf[l]), "(assert ");
      *remaining -= strlen(&(buf[l]));
      l += strlen(&(buf[l]));

      l += fmla_str(stmt->param.const_def->body, remaining, buf + l);

      buf[(*remaining)--, l++] = ')';
    } else {
      sprintf(&(buf[l]), "(define-fun %s (",
          stmt->param.const_def->const_name);
      *remaining -= strlen(&(buf[l]));
      l += strlen(&(buf[l]));

      struct terms_t * params_cursor = stmt->param.const_def->params;
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

void
free_stmt(struct stmt_t * stmt)
{
  switch (stmt->kind) {
  case STMT_AXIOM:
    free_fmla(stmt->param.axiom);
    break;
  case STMT_CONST_DEF:
    free((char *)stmt->param.const_def->const_name);
    if (NULL != stmt->param.const_def->params) {
      free_terms(stmt->param.const_def->params);
    }
    free_fmla(stmt->param.const_def->body);
    free(stmt->param.const_def);
    break;
  default:
    // FIXME complain
    break;
  }
  free(stmt);
}

struct stmts_t *
mk_stmt_cell(struct stmt_t * stmt, struct stmts_t * next)
{
  assert(NULL != stmt);

  struct stmts_t * ss = malloc(sizeof(struct stmts_t));
  assert(NULL != ss);

  ss->stmt = stmt;
  ss->next = next;
  return ss;
}

size_t
stmts_str(struct stmts_t * stmts, size_t * remaining, char * buf)
{
  size_t l = 0;
  struct stmts_t * cursor = stmts;
  while (NULL != cursor) {
    l += stmt_str(cursor->stmt, remaining, buf + l);
    buf[(*remaining)--, l++] = '\n';
    cursor = cursor->next;
  }
  return l;
}

void
free_stmts(struct stmts_t * stmts)
{
  assert(NULL != stmts->stmt);
  free_stmt(stmts->stmt);
  if (NULL != stmts->next) {
    free_stmts(stmts->next);
  }
  free(stmts);
}

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

  l += universe_str(mdl->universe, remaining, buf + l);
  l += stmts_str(mdl->stmts, remaining, buf + l);
  buf[l] = '\0';
  return l;
}

void
free_model(struct model_t * mdl)
{
  free_universe(mdl->universe);
  free_stmts(mdl->stmts);
  free(mdl);
}

void
strengthen_model(struct model_t * mdl, struct stmt_t * stmt)
{
  mdl->stmts = mk_stmt_cell(stmt, mdl->stmts);
}

void
test_statement()
{
  struct term_t * aT = mk_term(CONST, "a");
  struct term_t * bT = mk_term(CONST, "b");
  struct terms_t * terms = mk_term_cell(aT, NULL);
  terms = mk_term_cell(bT, terms);

  struct model_t * mdl = mk_model(mk_universe(terms));

  struct stmt_t * s1S = mk_stmt_axiom(mk_fmla_atom_varargs("=", 2, "a", "a"));
  char * vX = malloc(sizeof(char) * 2);
  strcpy(vX, "X");
  char * vY = malloc(sizeof(char) * 2);
  strcpy(vY, "Y");
  terms = mk_term_cell(mk_term(VAR, vX), NULL);
  terms = mk_term_cell(mk_term(VAR, vY), terms);
  struct stmt_t * s2S = mk_stmt_pred("some_predicate", terms, mk_fmla_not(mk_fmla_atom_varargs("=", 2, "X", "Y")));
  struct stmt_t * s3S = mk_stmt_const("x", mdl->universe, (const char * const)&universe_ty);

  strengthen_model(mdl, s1S);
  strengthen_model(mdl, s2S);
  strengthen_model(mdl, s3S);

  size_t remaining_buf_size = BUF_SIZE;
  char * buf = malloc(remaining_buf_size);
  size_t l = model_str(mdl, &remaining_buf_size, buf);
  printf("test model (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  free_model(mdl);
  free(buf);
}

struct stmts_t *
reverse_stmts(struct stmts_t * stmts)
{
  struct stmts_t * cursor = stmts;
  struct stmts_t * result = NULL;
  struct stmts_t * last = NULL;
  while (NULL != cursor) {
    assert(NULL != cursor->stmt);
    last = mk_stmt_cell(cursor->stmt, last);
    if (NULL == last->next) {
      result = last;
    }
    cursor = cursor->next;
  }
  free(stmts);
  return result;
}

struct term_t *
new_const_in_stmt(struct stmt_t * stmt)
{
  struct term_t * result = NULL;
  switch (stmt->kind) {
  case STMT_AXIOM:
    result = NULL;
    break;
  case STMT_CONST_DEF:
    // Non-Bool constants are all declared at the start.
    if (bool_ty == stmt->param.const_def->ty) {
      result = mk_term(CONST, stmt->param.const_def->const_name);
    } else {
      result = NULL;
    }
    break;
  default:
    // FIXME complain -- impossible result
    break;
  }
  return result;
}

struct terms_t *
consts_in_stmt(struct stmt_t * stmt)
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
