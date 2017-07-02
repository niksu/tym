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
term_to_str(const struct Term * const term, struct buffer_info * dst)
{
  assert(NULL != term);
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
predicate_to_str(const struct Atom * atom, struct buffer_info * dst)
{
  assert(NULL != atom);
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
atom_to_str(const struct Atom * const atom, struct buffer_info * dst)
{
  assert(NULL != atom);
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

struct Clause *
copy_clause(const struct Clause * const cp_clause)
{
  struct Clause * result = malloc(sizeof(struct Clause));
  struct Atom ** body = NULL;
  if (cp_clause->body_size > 0) {
    body = malloc(sizeof(struct Atom *) * cp_clause->body_size);
    for (int i = 0; i < cp_clause->body_size; i++) {
      body[i] = copy_atom(cp_clause->body[i]);
    }
  }
  *result = (struct Clause){
    .head = copy_atom(cp_clause->head),
    .body_size = cp_clause->body_size,
    .body = body
  };
  return result;
}

struct buffer_write_result *
clause_to_str(const struct Clause * const clause, struct buffer_info * dst)
{
  assert(NULL != clause);
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
program_to_str(const struct Program * const program, struct buffer_info * dst)
{
  assert(NULL != program);
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

struct Term *
mk_const(const char * cp_identifier)
{
  assert(NULL != cp_identifier);
  return mk_term(CONST, strdup(cp_identifier));
}

struct Term *
mk_var(const char * cp_identifier)
{
  assert(NULL != cp_identifier);
  return mk_term(VAR, strdup(cp_identifier));
}

struct Term *
mk_term(TermKind kind, const char * identifier)
{
  assert(NULL != identifier);

  struct Term * t = malloc(sizeof(struct Term));
  assert(NULL != t);

  t->kind = kind;
  t->identifier = identifier;
  return t;
}

DEFINE_MUTABLE_LIST_MK(term, term, struct Term, struct Terms)

DEFINE_U8_LIST_LEN(Terms)

struct Atom *
mk_atom(char * predicate, uint8_t arity, struct Terms * args) {
  assert(NULL != predicate);

  struct Atom * at = malloc(sizeof(struct Atom));
  assert(NULL != at);

  at->predicate = predicate;
  at->arity = arity;
  at->args = NULL;

  struct Terms * pre_position = NULL;

  if (at->arity > 0) {
    at->args = malloc(sizeof(struct Term *) * at->arity);
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

DEFINE_MUTABLE_LIST_MK(atom, atom, struct Atom, struct Atoms)

DEFINE_U8_LIST_LEN(Atoms)

struct Clause *
mk_clause(struct Atom * head, uint8_t body_size, struct Atoms * body) {
  assert(NULL != head);
  assert((NULL != body && body_size > 0) || (NULL == body && 0 == body_size));

  struct Clause * cl = malloc(sizeof(struct Clause));
  assert(NULL != cl);

  cl->head = head;
  cl->body_size = body_size;
  cl->body = NULL;

  struct Atoms * pre_position = NULL;

  if (cl->body_size > 0) {
    cl->body = malloc(sizeof(struct Atom *) * body_size);
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

DEFINE_MUTABLE_LIST_MK(clause, clause, struct Clause, struct Clauses)

DEFINE_U8_LIST_LEN(Clauses)

struct Program *
mk_program(uint8_t no_clauses, struct Clauses * program)
{
  struct Program * p = malloc(sizeof(struct Program));
  assert(NULL != p);

  p->no_clauses = no_clauses;

  struct Clauses * pre_position = NULL;

  if (no_clauses > 0) {
    p->program = malloc(sizeof(struct Clause *) * no_clauses);
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
free_term(struct Term * term)
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
free_terms(struct Terms * terms)
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
free_atom(struct Atom * at)
{
  assert(NULL != at);
  assert(NULL != at->predicate);

  DBG("Freeing atom: ");
  DBG_SYNTAX((void *)at, (x_to_str_t)atom_to_str);
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
free_atoms(struct Atoms * atoms)
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
free_clause(struct Clause * clause)
{
  assert(NULL != clause);

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
free_clauses(struct Clauses * clauses)
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
free_program(struct Program * program)
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
hash_term(const struct Term * term)
{
  assert(NULL != term);
  char result = hash_str(term->identifier);
  result ^= term->kind;
  return result;
}

char
hash_atom(const struct Atom * atom)
{
  assert(NULL != atom);

  char result = hash_str(atom->predicate);

  for (int i = 0; i < atom->arity; i++) {
    result = (char)((result * hash_term(atom->args[i])) % 256) - 128;
  }

  return result;
}

char
hash_clause(const struct Clause * clause) {
  assert(NULL != clause);

  char result = hash_atom(clause->head);

  for (int i = 0; i < clause->body_size; i++) {
    result ^= ((i + hash_atom(clause->body[i])) % 256) - 128;
  }

  return result;
}

bool
eq_term(const struct Term * const t1, const struct Term * const t2,
    enum eq_term_error * error_code, bool * result)
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
test_clause(void) {
  printf("***test_clause***\n");
  struct Term * t = malloc(sizeof(struct Term));
  *t = (struct Term){.kind = CONST, .identifier = strdup("ok")};

  struct Atom * at = malloc(sizeof(struct Atom));
  at->predicate = strdup("world");
  at->arity = 1;
  at->args = malloc(sizeof(struct Term *) * 1);
  at->args[0] = t;

  struct Atom * hd = malloc(sizeof(struct Atom));
  hd->predicate = strdup("hello");
  hd->arity = 0;
  hd->args = NULL;

  struct Clause * cl = malloc(sizeof(struct Clause));
  cl->head = hd;
  cl->body_size = 1;
  cl->body = malloc(sizeof(struct Atom *) * 1);
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

struct Term *
copy_term(const struct Term * const cp_term)
{
  assert(NULL != cp_term);
  return mk_term(cp_term->kind, strdup(cp_term->identifier));
}

// In practice, simply checks that ss is a subset of ts.
// FIXME naive implementation
bool
terms_subsumed_by(const struct Terms * const ts, const struct Terms * ss)
{
  if (NULL == ts) {
    return true;
  }

  bool result = true;

#if DEBUG
  printf("Subsumption check initially %d", result);
#endif
  while (NULL != ss) {
#if DEBUG
    printf(".");
#endif
    const struct Terms * cursor = ts;
    bool found = false;
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
terms_to_str(const struct Terms * const terms, struct buffer_info * dst)
{
  assert(NULL != terms);

  size_t initial_idx = dst->idx;

  const struct Terms * cursor = terms;

  struct buffer_write_result * res = NULL;

  while (NULL != cursor) {
    res = term_to_str(cursor->term, dst);
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

struct Atom *
copy_atom(const struct Atom * const cp_atom)
{
  assert(NULL != cp_atom);

  struct Atom * at = malloc(sizeof(struct Atom));
  assert(NULL != at);

  at->predicate = strdup(cp_atom->predicate);
  at->arity = cp_atom->arity;
  at->args = NULL;

  if (at->arity > 0) {
    at->args = malloc(sizeof(struct Term *) * at->arity);
    assert(NULL != at->args);
    for (int i = 0; i < at->arity; i++) {
      at->args[i] = copy_term(cp_atom->args[i]);
    }
  }

  return at;
}
