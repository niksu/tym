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
term_to_str(const struct term_t * const term, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, term->identifier);
  assert(is_ok_buffer_write_result(res));
  free(res);

  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

#if DEBUG
  char local_buf[BUF_SIZE];
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
predicate_to_str(const struct atom_t * atom, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, atom->predicate);
  assert(is_ok_buffer_write_result(res));
  free(res);

  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

#if DEBUG
  char local_buf[BUF_SIZE];
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
atom_to_str(const struct atom_t * const atom, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = predicate_to_str(atom, dst);
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
    res = term_to_str(atom->args[i], dst);
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

#if DEBUG
  char local_buf[BUF_SIZE];
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

struct buffer_write_result *
clause_to_str(const struct clause_t * const clause, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = atom_to_str(clause->head, dst);
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
      res = atom_to_str(clause->body[i], dst);
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

#if DEBUG
  char local_buf[BUF_SIZE];
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
program_to_str(const struct program_t * const program, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = NULL;

  for (int i = 0; i < program->no_clauses; i++) {
    res = clause_to_str(program->program[i], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    if (i < program->no_clauses - 1) {
      safe_buffer_replace_last(dst, '\n');
    }
  }

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct term_t *
mk_const(const char * cp_identifier)
{
  char * ident_copy = malloc(sizeof(char) * (strlen(cp_identifier) + 1));
  strcpy(ident_copy, cp_identifier);
  return mk_term(CONST, ident_copy);
}

struct term_t *
mk_var(const char * cp_identifier)
{
  char * ident_copy = malloc(sizeof(char) * (strlen(cp_identifier) + 1));
  strcpy(ident_copy, cp_identifier);
  return mk_term(VAR, ident_copy);
}

struct term_t *
mk_term(term_kind_t kind, const char * identifier)
{
  assert(NULL != identifier);

  struct term_t * t = malloc(sizeof(struct term_t));
  assert(NULL != t);

  t->kind = kind;
  t->identifier = identifier;
  return t;
}

DEFINE_MUTABLE_LIST_MK(term, term, struct term_t, struct terms_t)

DEFINE_U8_LIST_LEN(terms)

struct atom_t *
mk_atom(char * predicate, uint8_t arity, struct terms_t * args) {
  assert(NULL != predicate);

  struct atom_t * at = malloc(sizeof(struct atom_t));
  assert(NULL != at);

  at->predicate = predicate;
  at->arity = arity;
  at->args = NULL;

  struct terms_t * pre_position = NULL;

  if (at->arity > 0) {
    at->args = malloc(sizeof(struct term_t *) * at->arity);
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

DEFINE_MUTABLE_LIST_MK(atom, atom, struct atom_t, struct atoms_t)

DEFINE_U8_LIST_LEN(atoms)

struct clause_t *
mk_clause(struct atom_t * head, uint8_t body_size, struct atoms_t * body) {
  assert(NULL != head);

  struct clause_t * cl = malloc(sizeof(struct clause_t));
  assert(NULL != cl);

  cl->head = head;
  cl->body_size = body_size;

  struct atoms_t * pre_position = NULL;

  if (cl->body_size > 0) {
    cl->body = malloc(sizeof(struct atom_t *) * body_size);
    assert(NULL != cl->body);
    for (int i = 0; i < cl->body_size; i++) {
      cl->body[i] = body->atom;
      pre_position = body;
      body = body->next;
      free(pre_position);
    }
  } else {
    cl->body = NULL;
  }

  return cl;
}

DEFINE_MUTABLE_LIST_MK(clause, clause, struct clause_t, struct clauses_t)

DEFINE_U8_LIST_LEN(clauses)

struct program_t *
mk_program(uint8_t no_clauses, struct clauses_t * program)
{
  struct program_t * p = malloc(sizeof(struct program_t));
  assert(NULL != p);

  p->no_clauses = no_clauses;

  struct clauses_t * pre_position = NULL;

  if (no_clauses > 0) {
    p->program = malloc(sizeof(struct clause_t *) * no_clauses);
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
free_term(struct term_t * term)
{
  assert(NULL != term->identifier);

  free((void *)term->identifier);

  free((void *)term);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_terms(struct terms_t * terms)
{
  assert(NULL != terms);

  assert(NULL != terms->term);
  free_term(terms->term);
  if (NULL != terms->next) {
    free_terms((void *)terms->next);
  }
  free(terms);
}
#pragma GCC diagnostic pop

void
free_atom(struct atom_t * at)
{
  assert(NULL != at->predicate);

  DBG("Freeing atom: ");
  DBG_SYNTAX((void *)&at, (x_to_str_t)atom_to_str);
  DBG("\n");

  free(at->predicate);
  for (int i = 0; i < at->arity; i++) {
    free_term(at->args[i]);
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
free_atoms(struct atoms_t * atoms)
{
  assert(NULL != atoms);

  assert(NULL != atoms->atom);
  free_atom(atoms->atom);
  if (NULL != atoms->next) {
    free_atoms(atoms->next);
  }
  free(atoms);
}
#pragma GCC diagnostic pop

void
free_clause(struct clause_t * clause)
{
  free_atom(clause->head);

  assert((0 == clause->body_size && NULL == clause->body) ||
         (clause->body_size > 0 && NULL != clause->body));
  for (int i = 0; i < clause->body_size; i++) {
    free_atom(clause->body[i]);
  }

  if (clause->body_size > 0) {
    free(clause->body);
  }

  free(clause);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_clauses(struct clauses_t * clauses)
{
  assert(NULL != clauses);

  assert(NULL != clauses->clause);
  free_clause(clauses->clause);
  if (NULL != clauses->next) {
    free_clauses((void *)clauses->next);
  }
  free(clauses);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_program(struct program_t * program)
{
  assert(NULL != program);

  for (int i = 0; i < program->no_clauses; i++) {
    DBG("Freeing clause %d: ", i);
    DBG_SYNTAX((void *)program->program[i], (x_to_str_t)clause_to_str);
    DBG("\n");

    assert(NULL != (program->program[i]));
    free_clause(program->program[i]);
  }

  if (program->no_clauses > 0) {
    free(program->program); // Free the array of pointers to clauses.
  }

  free(program); // Free the program struct.
}
#pragma GCC diagnostic pop

void
debug_out_syntax(void * x, struct buffer_write_result * (*x_to_str)(void *, struct buffer_info * dst))
{
  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);

  struct buffer_write_result * res = x_to_str(x, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);

  DBG("%s", outbuf->buffer);

  free_buffer(outbuf);
}

char
hash_str(const char * str)
{
  char result = 0;
  const char * cursor;

  cursor = str;
  while ('\0' != *cursor) {
    result ^= *(cursor++);
  }

  return result;
}

char
hash_term(const struct term_t * term)
{
  char result = hash_str(term->identifier);
  result ^= term->kind;
  return result;
}

char
hash_atom(const struct atom_t * atom)
{
  char result = hash_str(atom->predicate);

  for (int i = 0; i < atom->arity; i++) {
    result = (char)((result * hash_term(atom->args[i])) % 256) - 128;
  }

  return result;
}

char
hash_clause(const struct clause_t * clause) {
  char result = hash_atom(clause->head);

  for (int i = 0; i < clause->body_size; i++) {
    result ^= ((i + hash_atom(clause->body[i])) % 256) - 128;
  }

  return result;
}

bool
eq_term(const struct term_t * const t1, const struct term_t * const t2,
    enum eq_term_error * error_code, bool * result)
{
  bool successful;

  bool same_kind = false;
  bool same_identifier = false;

  if (t1 == t2) {
    *result = true;
    return true;
  }

  if (t1->kind == t2->kind) {
    same_kind = true;
  }

  if (0 == strcmp(t1->identifier, t2->identifier)) {
    same_identifier = true;
  }

  if  (!same_kind && same_identifier) {
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
test_clause(void) {
  printf("***test_clause***\n");
  struct term_t * t = malloc(sizeof(struct term_t));
  *t = (struct term_t){.kind = CONST, .identifier = strdup("ok")};

  struct atom_t * at = malloc(sizeof(struct atom_t));
  at->predicate = strdup("world");
  at->arity = 1;
  at->args = malloc(sizeof(struct term_t *) * 1);
  at->args[0] = t;

  struct atom_t * hd = malloc(sizeof(struct atom_t));
  hd->predicate = strdup("hello");
  hd->arity = 0;
  hd->args = NULL;

  struct clause_t * cl = malloc(sizeof(struct clause_t));
  cl->head = hd;
  cl->body_size = 1;
  cl->body = malloc(sizeof(struct atom_t *) * 1);
  cl->body[0] = at;

  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = clause_to_str(cl, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test clause (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);
  free_buffer(outbuf);

  free_clause(cl);
}

struct term_t *
copy_term(const struct term_t * const cp_term)
{
  char * ident_copy = malloc(sizeof(char) * (strlen(cp_term->identifier) + 1));
  strcpy(ident_copy, cp_term->identifier);
  return mk_term(cp_term->kind, ident_copy);
}

// FIXME naive implementation
bool
terms_subsumed_by(const struct terms_t * const ts, const struct terms_t * ss)
{
  bool result = true;

#if DEBUG
  printf("Subsumption check initially %d", result);
#endif
  while (NULL != ss) {
#if DEBUG
    printf(".");
#endif
    const struct terms_t * cursor = ts;
    bool found = (NULL == cursor);
    enum eq_term_error error_code;
    while (NULL != cursor) {
      if (eq_term(cursor->term, ss->term, &error_code, &found)) {
        if (found) {
          break;
        }
      } else {
        assert(false);
      }
      cursor = cursor->next;
    }

    if (!found) {
#if DEBUG
      struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
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

#if DEBUG
  printf(" ultimately %d\n", result);
#endif

  return result;
}

struct buffer_write_result *
terms_to_str(const struct terms_t * const terms, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  const struct terms_t * cursor = terms;

  struct buffer_write_result * res = NULL;

  while (NULL != cursor) {
    res = term_to_str(cursor->term, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    if (NULL != terms->next) {
      if (have_space(dst, 2)) {
        unsafe_buffer_str(dst, ", ");
      } else {
        return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
      }
    }

    cursor = cursor->next;
  }

  unsafe_buffer_char(dst, '\0');

  return mkval_buffer_write_result(dst->idx - initial_idx);
}
