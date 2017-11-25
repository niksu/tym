/*
 * AST supporting functions.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "hash.h"
#include "module_tests.h"
#include "util.h"

struct TymTerm * tym_mdl_instantiate_valuation_term(struct TymTerm * term, struct TymMdlValuations * vals);
struct TymAtom * tym_mdl_instantiate_valuation_atom(struct TymAtom * atom, struct TymMdlValuations * vals);
struct TymClause * tym_mdl_instantiate_valuation_clause(struct TymClause * cl, struct TymMdlValuations * vals);

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_term_str(const struct TymTerm * const term, struct TymBufferInfo * dst)
{
  assert(NULL != term);
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_buf_strcpy(dst, tym_decode_str(term->identifier));
  assert(TYM_MAYBE_ERROR__IS_OK_FNAME(TymBufferWriteResult)(res));
  free(res);

  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.

#if TYM_DEBUG
  char local_buf[TYM_BUF_SIZE];
  sprintf(local_buf, "{hash=%d}", tym_hash_term(term));
  res = tym_buf_strcpy(dst, local_buf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.
#endif

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_predicate_atom_str(const struct TymAtom * atom, struct TymBufferInfo * dst)
{
  assert(NULL != atom);
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_buf_strcpy(dst, tym_decode_str(atom->predicate));
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.

#if TYM_DEBUG
  char local_buf[TYM_BUF_SIZE];
  sprintf(local_buf, "{hash=%u}", tym_hash_str(tym_decode_str(atom->predicate)));
  res = tym_buf_strcpy(dst, local_buf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.
#endif

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_atom_str(const struct TymAtom * const atom, struct TymBufferInfo * dst)
{
  assert(NULL != atom);
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_predicate_atom_str(atom, dst);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.

  // There needs to be space to store at least "()\0".
  if (tym_have_space(dst, 3)) {
    tym_unsafe_buffer_char(dst, '(');
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }

  for (int i = 0; i < atom->arity; i++) {
    res = tym_term_str(atom->args[i], dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.

    if (i != atom->arity - 1) {
      // There needs to be space to store at least ", x)\0".
      if (!tym_have_space(dst, 5)) {
        return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
      }

      tym_unsafe_buffer_str(dst, ", ");
    }
  }

  if (tym_have_space(dst, 2)) {
    tym_unsafe_buffer_char(dst, ')');
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }

#if TYM_DEBUG
  char local_buf[TYM_BUF_SIZE];
  sprintf(local_buf, "{hash=%d}", tym_hash_atom(atom));
  res = tym_buf_strcpy(dst, local_buf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.
#endif

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

struct TymClause *
tym_copy_clause(const struct TymClause * const cp_clause)
{
  struct TymClause * result = malloc(sizeof *result);
  struct TymAtom ** body = NULL;
  if (cp_clause->body_size > 0) {
    body = malloc(sizeof *body * cp_clause->body_size);
    for (int i = 0; i < cp_clause->body_size; i++) {
      body[i] = tym_copy_atom(cp_clause->body[i]);
    }
  }
  *result = (struct TymClause){
    .head = tym_copy_atom(cp_clause->head),
    .body_size = cp_clause->body_size,
    .body = body
  };
  return result;
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_clause_str(const struct TymClause * const clause, struct TymBufferInfo * dst)
{
  assert(NULL != clause);
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_atom_str(clause->head, dst);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.

  if (clause->body_size > 0) {
    // There needs to be space to store at least " :- x().".
    if (!tym_have_space(dst, 8)) {
      return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
    }

    tym_unsafe_buffer_str(dst, " :- ");

    for (int i = 0; i < clause->body_size; i++) {
      res = tym_atom_str(clause->body[i], dst);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);

      tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.

      if (i != clause->body_size - 1) {
        // There needs to be space to store at least ", x().\0".
        if (!tym_have_space(dst, 7)) {
          return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
        }

        tym_unsafe_buffer_str(dst, ", ");
      }
    }
  }

  // There needs to be space to store at least ".\0".
  if (tym_have_space(dst, 2)) {
    tym_unsafe_buffer_char(dst, '.');
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }

#if TYM_DEBUG
  char local_buf[TYM_BUF_SIZE];
  sprintf(local_buf, "{hash=%d}", tym_hash_clause(clause));
  res = tym_buf_strcpy(dst, local_buf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.
#endif

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_program_str(const struct TymProgram * const program, struct TymBufferInfo * dst)
{
  assert(NULL != program);
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  for (int i = 0; i < program->no_clauses; i++) {
    res = tym_clause_str(program->program[i], dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    if (i < program->no_clauses - 1) {
      tym_safe_buffer_replace_last(dst, '\n');
    }
  }

  return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
}

struct TymTerm *
tym_mk_term(enum TymTermKind kind, const TymStr * identifier)
{
  assert(NULL != identifier);
  assert(TYM_CONST == kind || TYM_VAR == kind || TYM_STR == kind);

  struct TymTerm * t = malloc(sizeof *t);
  assert(NULL != t);

  t->kind = kind;
  t->identifier = identifier;
  return t;
}

TYM_DEFINE_MUTABLE_LIST_MK(term, term, struct TymTerm, struct TymTerms)

TYM_DEFINE_U8_LIST_LEN(TymTerms)

struct TymAtom *
tym_mk_atom(TymStr * predicate, uint8_t arity, struct TymTerms * args) {
  assert(NULL != predicate);

  struct TymAtom * at = malloc(sizeof *at);
  assert(NULL != at);

  at->predicate = predicate;
  at->arity = arity;
  at->args = NULL;

  struct TymTerms * pre_position = NULL;

  if (at->arity > 0) {
    at->args = malloc(sizeof *at->args * at->arity);
    assert(NULL != at->args);
    for (int i = 0; i < at->arity; i++) {
      at->args[i] = args->term;
      pre_position = args;
      args = args->next;
      free(pre_position);
    }
  }

  return at;
}

TYM_DEFINE_MUTABLE_LIST_MK(atom, atom, struct TymAtom, struct TymAtoms)

TYM_DEFINE_U8_LIST_LEN(TymAtoms)

struct TymClause *
tym_mk_clause(struct TymAtom * head, uint8_t body_size, struct TymAtoms * body) {
  assert(NULL != head);
  assert((NULL != body && body_size > 0) || (NULL == body && 0 == body_size));

  struct TymClause * cl = malloc(sizeof *cl);
  assert(NULL != cl);

  cl->head = head;
  cl->body_size = body_size;
  cl->body = NULL;

  struct TymAtoms * pre_position = NULL;

  if (cl->body_size > 0) {
    cl->body = malloc(sizeof *cl->body * body_size);
    assert(NULL != cl->body);
    for (int i = 0; i < cl->body_size; i++) {
      cl->body[i] = body->atom;
      pre_position = body;
      body = body->next;
      free(pre_position);
    }
  }

  return cl;
}

TYM_DEFINE_MUTABLE_LIST_MK(clause, clause, struct TymClause, struct TymClauses)

TYM_DEFINE_U8_LIST_LEN(TymClauses)

struct TymProgram *
tym_mk_program(uint8_t no_clauses, struct TymClauses * program)
{
  struct TymProgram * p = malloc(sizeof *p);
  assert(NULL != p);

  p->no_clauses = no_clauses;

  struct TymClauses * pre_position = NULL;

  if (no_clauses > 0) {
    p->program = malloc(sizeof *p->program * no_clauses);
    assert(NULL != p->program);
    for (int i = 0; i < p->no_clauses; i++) {
      p->program[i] = program->clause;
      pre_position = program;
      program = program->next;
      free(pre_position);
    }
  } else {
    p->program = NULL;
  }

  return p;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_term(struct TymTerm * term)
{
  assert(NULL != term);

  assert(NULL != term->identifier);

  tym_free_str(term->identifier);

  free((void *)term);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_terms(struct TymTerms * terms)
{
  assert(NULL != terms);

  assert(NULL != terms->term);
  tym_free_term(terms->term);
  if (NULL != terms->next) {
    tym_free_terms((void *)terms->next);
  }
  free(terms);
}
#pragma GCC diagnostic pop

void
tym_free_atom(struct TymAtom * at)
{
  assert(NULL != at);
  assert(NULL != at->predicate);

  TYM_DBG("Freeing atom: ");
  TYM_DBG_SYNTAX((void *)at, (tym_x_str_t)tym_atom_str);
  TYM_DBG("\n");

  tym_free_str(at->predicate);
  for (int i = 0; i < at->arity; i++) {
    tym_free_term(at->args[i]);
  }

  if (at->arity > 0) {
    // Since we allocated the space for all arguments, rather than for each argument,
    // we deallocate it as such.
    free(at->args);
  }

  free(at);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_atoms(struct TymAtoms * atoms)
{
  assert(NULL != atoms);

  assert(NULL != atoms->atom);
  tym_free_atom(atoms->atom);
  if (NULL != atoms->next) {
    tym_free_atoms(atoms->next);
  }
  free(atoms);
}
#pragma GCC diagnostic pop

void
tym_free_clause(struct TymClause * clause)
{
  assert(NULL != clause);

  tym_free_atom(clause->head);

  assert((0 == clause->body_size && NULL == clause->body) ||
         (clause->body_size > 0 && NULL != clause->body));
  for (int i = 0; i < clause->body_size; i++) {
    tym_free_atom(clause->body[i]);
  }

  if (clause->body_size > 0) {
    free(clause->body);
  }

  free(clause);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_clauses(struct TymClauses * clauses)
{
  assert(NULL != clauses);

  assert(NULL != clauses->clause);
  tym_free_clause(clauses->clause);
  if (NULL != clauses->next) {
    tym_free_clauses((void *)clauses->next);
  }
  free(clauses);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_program(struct TymProgram * program)
{
  assert(NULL != program);

  for (int i = 0; i < program->no_clauses; i++) {
    TYM_DBG("Freeing clause %d: ", i);
    TYM_DBG_SYNTAX((void *)program->program[i], (tym_x_str_t)tym_clause_str);
    TYM_DBG("\n");

    assert(NULL != (program->program[i]));
    tym_free_clause(program->program[i]);
  }

  if (program->no_clauses > 0) {
    free(program->program); // Free the array of pointers to clauses.
  }

  free(program); // Free the program struct.
}
#pragma GCC diagnostic pop

void
tym_debug_out_syntax(void * x, struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * (*tym_x_str)(void *, struct TymBufferInfo * dst))
{
  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_x_str(x, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  TYM_DBG("%s", tym_buffer_contents(outbuf));

  tym_free_buffer(outbuf);
}

TYM_HASH_VTYPE
tym_hash_term(const struct TymTerm * term)
{
  assert(NULL != term);
  TYM_HASH_VTYPE result = tym_hash_str(tym_decode_str(term->identifier));
  result ^= (TYM_HASH_VTYPE)term->kind;
  return result;
}

TYM_HASH_VTYPE
tym_hash_atom(const struct TymAtom * atom)
{
  assert(NULL != atom);

  TYM_HASH_VTYPE result = tym_hash_str(tym_decode_str(atom->predicate));

  result ^= (TYM_HASH_VTYPE)atom->arity;

  for (int i = 0; i < atom->arity; i++) {
    result ^= (TYM_HASH_VTYPE)((i + 1) * tym_hash_term(atom->args[i]));
  }

  return result;
}

TYM_HASH_VTYPE
tym_hash_clause(const struct TymClause * clause) {
  assert(NULL != clause);

  TYM_HASH_VTYPE result = tym_hash_atom(clause->head);

  result ^= (TYM_HASH_VTYPE)clause->body_size;

  for (int i = 0; i < clause->body_size; i++) {
    result ^= (TYM_HASH_VTYPE)((i + 1) * tym_hash_atom(clause->body[i]));
  }

  return result;
}

bool
tym_eq_term(const struct TymTerm * const t1, const struct TymTerm * const t2,
    enum TymEqTermError * error_code, bool * result)
{
  assert(NULL != t1);
  assert(NULL != t2);

  bool successful;

  bool same_kind = false;
  bool same_identifier = false;

  if (t1 == t2) {
    *error_code = TYM_NO_ERROR;
    *result = true;
    return true;
  }

  if (t1->kind == t2->kind) {
    same_kind = true;
  }

  if (0 == tym_cmp_str(t1->identifier, t2->identifier)) {
    same_identifier = true;
  }

  if (!same_kind && same_identifier) {
    successful = false;
    *error_code = TYM_DIFF_KIND_SAME_IDENTIFIER;
  } else {
    successful = true;
    *error_code = TYM_NO_ERROR;
    *result = same_identifier;
  }

  return successful;
}

void
tym_test_clause(void) {
  printf("***test_clause***\n");
  struct TymTerm * t = malloc(sizeof *t);
  *t = (struct TymTerm){.kind = TYM_CONST,
    .identifier = TYM_CSTR_DUPLICATE("ok")};

  struct TymAtom * at = malloc(sizeof *at);
  at->predicate = TYM_CSTR_DUPLICATE("world");
  at->arity = 1;
  at->args = malloc(sizeof *at->args * 1);
  at->args[0] = t;

  struct TymAtom * hd = malloc(sizeof *hd);
  hd->predicate = TYM_CSTR_DUPLICATE("hello");
  hd->arity = 0;
  hd->args = NULL;

  struct TymClause * cl = malloc(sizeof *cl);
  cl->head = hd;
  cl->body_size = 1;
  cl->body = malloc(sizeof *cl->body * 1);
  cl->body[0] = at;

  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_clause_str(cl, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  TYM_DBG_BUFFER(outbuf, "test clause")
  tym_free_buffer(outbuf);

  tym_free_clause(cl);
}

struct TymTerm *
tym_copy_term(const struct TymTerm * const cp_term)
{
  assert(NULL != cp_term);
  return tym_mk_term(cp_term->kind,
      TYM_STR_DUPLICATE(cp_term->identifier));
}

// In practice, simply checks that ss is a subset of ts.
// FIXME naive implementation
bool
tym_terms_subsumed_by(const struct TymTerms * const ts, const struct TymTerms * ss)
{
  if (NULL == ts) {
    return true;
  }

  bool result = true;

#if TYM_DEBUG
  printf("Subsumption check initially %d", result);
#endif
  while (NULL != ss) {
#if TYM_DEBUG
    printf(".");
#endif
    const struct TymTerms * cursor = ts;
    bool found = false;
    enum TymEqTermError error_code;
    while (NULL != cursor) {
      if (tym_eq_term(cursor->term, ss->term, &error_code, &found)) {
        if (found) {
          break;
        }
      } else {
        assert(false);
      }
      cursor = cursor->next;
    }

    if (!found) {
#if TYM_DEBUG
      struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
      struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_term_str(ss->term, outbuf);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
      TYM_DBG_BUFFER(outbuf, "unsubsumed")
      tym_free_buffer(outbuf);
#endif
      result = false;
      break;
    }

    ss = ss->next;
  }

#if TYM_DEBUG
  printf(" ultimately %d\n", result);
#endif

  return result;
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_terms_str(const struct TymTerms * const terms, struct TymBufferInfo * dst)
{
  assert(NULL != terms);

  size_t initial_idx = tym_buffer_len(dst);

  const struct TymTerms * cursor = terms;

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  while (NULL != cursor) {
    res = tym_term_str(cursor->term, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    if (NULL != terms->next) {
      tym_safe_buffer_replace_last(dst, ',');
      if (tym_have_space(dst, 1)) {
        tym_unsafe_buffer_char(dst, ' ');
      } else {
        return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
      }
    }

    cursor = cursor->next;
  }

  tym_unsafe_buffer_char(dst, '\0');

  return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
}

struct TymAtom *
tym_copy_atom(const struct TymAtom * const cp_atom)
{
  assert(NULL != cp_atom);

  struct TymAtom * at = malloc(sizeof *at);
  assert(NULL != at);

  at->predicate = TYM_STR_DUPLICATE(cp_atom->predicate);
  at->arity = cp_atom->arity;
  at->args = NULL;

  if (at->arity > 0) {
    at->args = malloc(sizeof *at->args * at->arity);
    assert(NULL != at->args);
    for (int i = 0; i < at->arity; i++) {
      at->args[i] = tym_copy_term(cp_atom->args[i]);
    }
  }

  return at;
}

bool
// FIXME this function can be made generic wrt the set, and made more efficient.
tym_vars_contained(const struct TymTerm * e, struct TymTerms * set)
{
  while (NULL != set) {
    enum TymEqTermError error_code;
    bool result;
    if (tym_eq_term(set->term, e, &error_code, &result)) {
      if (result) {
        return true;
      }
    } else {
      assert(false);
    }
    set = set->next;
  }
  return false;
}

struct TymTerms *
// FIXME this function can be made generic wrt the list.
tym_terms_difference(struct TymTerms * set1, struct TymTerms * set2)
{
  struct TymTerms * result = NULL;
  while (NULL != set1) {
    if (!tym_vars_contained(set1->term, set2)) {
      result = tym_mk_term_cell(set1->term, result);
    }
    set1 = set1->next;
  }
  return result;
}

void
tym_vars_of_atom(struct TymAtom * atom, struct TymTerms ** acc)
{
  for (int i = 0; i < atom->arity; i++) {
    if ((TYM_VAR == atom->args[i]->kind) &&
        (!tym_vars_contained(atom->args[i], *acc))) {
      *acc = tym_mk_term_cell(atom->args[i], *acc);
    }
  }
}

struct TymTerms *
tym_hidden_vars_of_clause(const struct TymClause * cl)
{
  struct TymTerms * head_vars = NULL;
  struct TymTerms * body_vars = NULL;
  tym_vars_of_atom(cl->head, &head_vars);
  for (int i = 0; i < cl->body_size; i++) {
    tym_vars_of_atom(cl->body[i], &body_vars);
  }

  struct TymTerms * result = tym_terms_difference(body_vars, head_vars);
  tym_shallow_free_terms(head_vars);
  tym_shallow_free_terms(body_vars);
  return result;
}

TYM_DEFINE_LIST_SHALLOW_FREE(terms, , struct TymTerms)

struct TymMdlValuations *
tym_mdl_mk_valuations(const TymStr ** consts, const TymStr ** vars)
{
  assert(NULL != consts);
  assert(NULL != vars);
  unsigned count = 0;
  while (NULL != consts[count]) {
    count++;
  }

  {
    unsigned var_count = 0;
    while (NULL != vars[var_count]) {
      var_count++;
    }
    assert(count == var_count);
  }

  struct TymMdlValuations * result = malloc(sizeof(*result));
  result->count = count;
  result->v = malloc(sizeof(*result->v) * count);
  for (unsigned i = 0; i < count; i++) {
    result->v[i].const_name = TYM_STR_DUPLICATE(consts[i]);
    result->v[i].var_name = TYM_STR_DUPLICATE(vars[i]);
    result->v[i].value = NULL;
  }

  return result;
}

void
tym_mdl_free_valuations(struct TymMdlValuations * vals)
{
  assert(NULL != vals);
  for (unsigned i = 0; i < vals->count; i++) {
    tym_free_str(vals->v[i].const_name);
    tym_free_str(vals->v[i].var_name);
    if (NULL != vals->v[i].value) {
      tym_free_str(vals->v[i].value);
    }
  }
  free(vals->v);
  free(vals);
}

void
tym_mdl_print_valuations(const struct TymMdlValuations * vals)
{
  assert(NULL != vals);
  for (unsigned i = 0; i < vals->count; i++) {
    printf("%s = %s", tym_decode_str(vals->v[i].var_name), tym_decode_str(vals->v[i].value));
    if (i < vals->count - 1) {
      printf(", ");
    }
  }
  printf("\n");
}

void
tym_mdl_reset_valuations(struct TymMdlValuations * vals)
{
  for (unsigned i = 0; i < vals->count; i++) {
    tym_free_str(vals->v[i].value);
    vals->v[i].value = NULL;
  }
}

struct TymTerm *
tym_mdl_instantiate_valuation_term(struct TymTerm * term, struct TymMdlValuations * vals)
{
  struct TymTerm * result = NULL;
  if (TYM_VAR == term->kind) {
    // FIXME inefficient -- linear time.
    for (unsigned i = 0; i < vals->count; i++) {
      if (0 == tym_cmp_str(vals->v[i].var_name, term->identifier)) {
        result = malloc(sizeof(*result));
        result->identifier = TYM_STR_DUPLICATE(vals->v[i].value);
        result->kind = TYM_CONST;
      }
    }
  }

  if (NULL == result) {
    result = tym_copy_term(term);
  }

  return result;
}

struct TymAtom *
tym_mdl_instantiate_valuation_atom(struct TymAtom * atom, struct TymMdlValuations * vals)
{
  struct TymAtom * result = malloc(sizeof(*result));
  result->predicate = TYM_STR_DUPLICATE(atom->predicate);
  result->arity = atom->arity;
  result->args = NULL;
  if (result->arity > 0) {
    result->args = malloc(sizeof(*(result->args)) * result->arity);
  }
  for (int i = 0; i < atom->arity; i++) {
    result->args[i] = tym_mdl_instantiate_valuation_term(atom->args[i], vals);
  }
  return result;
}

struct TymClause *
tym_mdl_instantiate_valuation_clause(struct TymClause * cl, struct TymMdlValuations * vals)
{
  struct TymClause * result = malloc(sizeof(*result));
  result->head = tym_mdl_instantiate_valuation_atom(cl->head, vals);
  result->body_size = cl->body_size;
  result->body = NULL;
  if (result->body_size > 0) {
    result->body = malloc(sizeof(*(result->body)) * result->body_size);
  }
  for (int i = 0; i < cl->body_size; i++) {
    result->body[i] = tym_mdl_instantiate_valuation_atom(cl->body[i], vals);
  }
  return result;
}

struct TymProgram *
tym_mdl_instantiate_valuation(struct TymProgram * ParsedQuery, struct TymMdlValuations * vals)
{
// FIXME Move tym_mdl_* to AST -- or some derivative of that.
//  since we're working with AST, not with formulas (so formula.c+h isn't the right place for tym_mdl_*)

  struct TymProgram * result = malloc(sizeof(*result));
  result->no_clauses = ParsedQuery->no_clauses;
  result->program = malloc(sizeof(*(result->program)) * result->no_clauses);
  for (int i = 0; i < ParsedQuery->no_clauses; i++) {
    result->program[i] = tym_mdl_instantiate_valuation_clause(ParsedQuery->program[i], vals);
  }
  return result;
}
