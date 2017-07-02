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
#include "module_tests.h"
#include "util.h"

struct buffer_write_result *
tym_term_to_str(const struct TymTerm * const term, struct buffer_info * dst)
{
  assert(NULL != term);
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, term->identifier);
  assert(is_ok_buffer_write_result(res));
  free(res);

  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

#if TYM_DEBUG
  char local_buf[TYM_BUF_SIZE];
  sprintf(local_buf, "{hash=%d}", hash_term(term) + 127);
  res = buf_strcpy(dst, local_buf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  unsafe_dec_idx(dst, 1); // chomp the trailing \0.
#endif

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
}

struct buffer_write_result *
tym_predicate_to_str(const struct TymAtom * atom, struct buffer_info * dst)
{
  assert(NULL != atom);
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, atom->predicate);
  assert(is_ok_buffer_write_result(res));
  free(res);

  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

#if TYM_DEBUG
  char local_buf[TYM_BUF_SIZE];
  sprintf(local_buf, "{hash=%d}", hash_str(atom->predicate) + 127);
  res = buf_strcpy(dst, local_buf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  unsafe_dec_idx(dst, 1); // chomp the trailing \0.
#endif

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
}

struct buffer_write_result *
tym_atom_to_str(const struct TymAtom * const atom, struct buffer_info * dst)
{
  assert(NULL != atom);
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = tym_predicate_to_str(atom, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

  // There needs to be space to store at least "()\0".
  if (have_space(dst, 3)) {
    unsafe_buffer_char(dst, '(');
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }

  for (int i = 0; i < atom->arity; i++) {
    res = tym_term_to_str(atom->args[i], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    unsafe_dec_idx(dst, 1); // chomp the trailing \0.

    if (i != atom->arity - 1) {
      // There needs to be space to store at least ", x)\0".
      if (!have_space(dst, 5)) {
        return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
      }

      unsafe_buffer_str(dst, ", ");
    }
  }

  if (have_space(dst, 2)) {
    unsafe_buffer_char(dst, ')');
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }

#if TYM_DEBUG
  char local_buf[TYM_BUF_SIZE];
  sprintf(local_buf, "{hash=%d}", hash_atom(atom) + 127);
  res = buf_strcpy(dst, local_buf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  unsafe_dec_idx(dst, 1); // chomp the trailing \0.
#endif

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
}

struct TymClause *
tym_copy_clause(const struct TymClause * const cp_clause)
{
  struct TymClause * result = malloc(sizeof(struct TymClause));
  struct TymAtom ** body = NULL;
  if (cp_clause->body_size > 0) {
    body = malloc(sizeof(struct TymAtom *) * cp_clause->body_size);
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

struct buffer_write_result *
tym_clause_to_str(const struct TymClause * const clause, struct buffer_info * dst)
{
  assert(NULL != clause);
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = tym_atom_to_str(clause->head, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

  if (clause->body_size > 0) {
    // There needs to be space to store at least " :- x().".
    if (!have_space(dst, 8)) {
      return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
    }

    unsafe_buffer_str(dst, " :- ");

    for (int i = 0; i < clause->body_size; i++) {
      res = tym_atom_to_str(clause->body[i], dst);
      assert(is_ok_buffer_write_result(res));
      free(res);

      unsafe_dec_idx(dst, 1); // chomp the trailing \0.

      if (i != clause->body_size - 1) {
        // There needs to be space to store at least ", x().\0".
        if (!have_space(dst, 7)) {
          return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
        }

        unsafe_buffer_str(dst, ", ");
      }
    }
  }

  // There needs to be space to store at least ".\0".
  if (have_space(dst, 2)) {
    unsafe_buffer_char(dst, '.');
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }

#if TYM_DEBUG
  char local_buf[TYM_BUF_SIZE];
  sprintf(local_buf, "{hash=%d}", hash_clause(clause) + 127);
  res = buf_strcpy(dst, local_buf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  unsafe_dec_idx(dst, 1); // chomp the trailing \0.
#endif

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
}

struct buffer_write_result *
tym_program_to_str(const struct TymProgram * const program, struct buffer_info * dst)
{
  assert(NULL != program);
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = NULL;

  for (int i = 0; i < program->no_clauses; i++) {
    res = tym_clause_to_str(program->program[i], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    if (i < program->no_clauses - 1) {
      safe_buffer_replace_last(dst, '\n');
    }
  }

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct TymTerm *
tym_mk_const(const char * cp_identifier)
{
  assert(NULL != cp_identifier);
  return tym_mk_term(CONST, strdup(cp_identifier));
}

struct TymTerm *
tym_mk_var(const char * cp_identifier)
{
  assert(NULL != cp_identifier);
  return tym_mk_term(VAR, strdup(cp_identifier));
}

struct TymTerm *
tym_mk_term(TymTermKind kind, const char * identifier)
{
  assert(NULL != identifier);

  struct TymTerm * t = malloc(sizeof(struct TymTerm));
  assert(NULL != t);

  t->kind = kind;
  t->identifier = identifier;
  return t;
}

TYM_DEFINE_MUTABLE_LIST_MK(term, term, struct TymTerm, struct TymTerms)

TYM_DEFINE_U8_LIST_LEN(TymTerms)

struct TymAtom *
tym_mk_atom(char * predicate, uint8_t arity, struct TymTerms * args) {
  assert(NULL != predicate);

  struct TymAtom * at = malloc(sizeof(struct TymAtom));
  assert(NULL != at);

  at->predicate = predicate;
  at->arity = arity;
  at->args = NULL;

  struct TymTerms * pre_position = NULL;

  if (at->arity > 0) {
    at->args = malloc(sizeof(struct TymTerm *) * at->arity);
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

  struct TymClause * cl = malloc(sizeof(struct TymClause));
  assert(NULL != cl);

  cl->head = head;
  cl->body_size = body_size;
  cl->body = NULL;

  struct TymAtoms * pre_position = NULL;

  if (cl->body_size > 0) {
    cl->body = malloc(sizeof(struct TymAtom *) * body_size);
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
  struct TymProgram * p = malloc(sizeof(struct TymProgram));
  assert(NULL != p);

  p->no_clauses = no_clauses;

  struct TymClauses * pre_position = NULL;

  if (no_clauses > 0) {
    p->program = malloc(sizeof(struct TymClause *) * no_clauses);
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

  free((void *)term->identifier);

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
  TYM_DBG_SYNTAX((void *)at, (tym_x_to_str_t)tym_atom_to_str);
  TYM_DBG("\n");

  free(at->predicate);
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
    TYM_DBG_SYNTAX((void *)program->program[i], (tym_x_to_str_t)clause_to_str);
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
tym_debug_out_syntax(void * x, struct buffer_write_result * (*tym_x_to_str)(void *, struct buffer_info * dst))
{
  struct buffer_info * outbuf = mk_buffer(TYM_BUF_SIZE);

  struct buffer_write_result * res = tym_x_to_str(x, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);

  TYM_DBG("%s", outbuf->buffer);

  free_buffer(outbuf);
}

char
tym_hash_str(const char * str)
{
  assert(NULL != str);

  char result = 0;
  const char * cursor;

  cursor = str;
  while ('\0' != *cursor) {
    result ^= *(cursor++);
  }

  return result;
}

char
tym_hash_term(const struct TymTerm * term)
{
  assert(NULL != term);
  char result = tym_hash_str(term->identifier);
  result ^= term->kind;
  return result;
}

char
tym_hash_atom(const struct TymAtom * atom)
{
  assert(NULL != atom);

  char result = tym_hash_str(atom->predicate);

  for (int i = 0; i < atom->arity; i++) {
    result = (char)((result * tym_hash_term(atom->args[i])) % 256) - 128;
  }

  return result;
}

char
tym_hash_clause(const struct TymClause * clause) {
  assert(NULL != clause);

  char result = tym_hash_atom(clause->head);

  for (int i = 0; i < clause->body_size; i++) {
    result ^= ((i + tym_hash_atom(clause->body[i])) % 256) - 128;
  }

  return result;
}

bool
tym_eq_term(const struct TymTerm * const t1, const struct TymTerm * const t2,
    enum tym_eq_term_error * error_code, bool * result)
{
  assert(NULL != t1);
  assert(NULL != t2);

  bool successful;

  bool same_kind = false;
  bool same_identifier = false;

  if (t1 == t2) {
    *error_code = NO_ERROR;
    *result = true;
    return true;
  }

  if (t1->kind == t2->kind) {
    same_kind = true;
  }

  if (0 == strcmp(t1->identifier, t2->identifier)) {
    same_identifier = true;
  }

  if (!same_kind && same_identifier) {
    successful = false;
    *error_code = DIFF_KIND_SAME_IDENTIFIER;
  } else {
    successful = true;
    *error_code = NO_ERROR;
    *result = same_identifier;
  }

  return successful;
}

void
tym_test_clause(void) {
  printf("***test_clause***\n");
  struct TymTerm * t = malloc(sizeof(struct TymTerm));
  *t = (struct TymTerm){.kind = CONST, .identifier = strdup("ok")};

  struct TymAtom * at = malloc(sizeof(struct TymAtom));
  at->predicate = strdup("world");
  at->arity = 1;
  at->args = malloc(sizeof(struct TymTerm *) * 1);
  at->args[0] = t;

  struct TymAtom * hd = malloc(sizeof(struct TymAtom));
  hd->predicate = strdup("hello");
  hd->arity = 0;
  hd->args = NULL;

  struct TymClause * cl = malloc(sizeof(struct TymClause));
  cl->head = hd;
  cl->body_size = 1;
  cl->body = malloc(sizeof(struct TymAtom *) * 1);
  cl->body[0] = at;

  struct buffer_info * outbuf = mk_buffer(TYM_BUF_SIZE);
  struct buffer_write_result * res = tym_clause_to_str(cl, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test clause (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);
  free_buffer(outbuf);

  tym_free_clause(cl);
}

struct TymTerm *
tym_copy_term(const struct TymTerm * const cp_term)
{
  assert(NULL != cp_term);
  return tym_mk_term(cp_term->kind, strdup(cp_term->identifier));
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
    enum tym_eq_term_error error_code;
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
      struct buffer_info * outbuf = mk_buffer(TYM_BUF_SIZE);
      struct buffer_write_result * res = term_to_str(ss->term, outbuf);
      assert(is_ok_buffer_write_result(res));
      free(res);
      printf("unsubsumed (size=%zu, remaining=%zu)\n|%s|\n",
          outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
      free_buffer(outbuf);
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

struct buffer_write_result *
tym_terms_to_str(const struct TymTerms * const terms, struct buffer_info * dst)
{
  assert(NULL != terms);

  size_t initial_idx = dst->idx;

  const struct TymTerms * cursor = terms;

  struct buffer_write_result * res = NULL;

  while (NULL != cursor) {
    res = tym_term_to_str(cursor->term, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    if (NULL != terms->next) {
      safe_buffer_replace_last(dst, ',');
      if (have_space(dst, 1)) {
        unsafe_buffer_char(dst, ' ');
      } else {
        return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
      }
    }

    cursor = cursor->next;
  }

  unsafe_buffer_char(dst, '\0');

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct TymAtom *
tym_copy_atom(const struct TymAtom * const cp_atom)
{
  assert(NULL != cp_atom);

  struct TymAtom * at = malloc(sizeof(struct TymAtom));
  assert(NULL != at);

  at->predicate = strdup(cp_atom->predicate);
  at->arity = cp_atom->arity;
  at->args = NULL;

  if (at->arity > 0) {
    at->args = malloc(sizeof(struct TymTerm *) * at->arity);
    assert(NULL != at->args);
    for (int i = 0; i < at->arity; i++) {
      at->args[i] = tym_copy_term(cp_atom->args[i]);
    }
  }

  return at;
}
