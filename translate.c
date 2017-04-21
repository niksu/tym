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
