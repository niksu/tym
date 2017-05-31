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

#if 0
size_t
my_strcpy(char * dst, const char * src, size_t * space)
{
  size_t l = strlen(src);
  if (l < *space) {
    strcpy(dst, src);
  } else {
    // FIXME complain
  }

  *space -= l;
  return l;
}
#endif
// FIXME this is simply a wrapper for the B-version of the function.
size_t
my_strcpy(char * dst, const char * src, size_t * space)
{
  struct buffer_info b =
    {
      .buffer = dst,
      .idx = 0,
      .buffer_size = *space
    };

  struct buffer_write_result * res = buf_strcpy(&b, src);
  assert(is_ok_buffer_write_result(res));
  free(res);

  size_t l = b.idx;

  return l;
}

#if 0
size_t
term_to_str(const struct term_t * const term, size_t * outbuf_size, char * outbuf)
{
  size_t l = my_strcpy(outbuf, term->identifier, outbuf_size); // FIXME add error checking

#if DEBUG
  sprintf(&(outbuf[l]), "{hash=%d}", hash_term(*term) + 127);
  *outbuf_size -= strlen(&(outbuf[l]));
  l += strlen(&(outbuf[l]));
#endif
  outbuf[l] = '\0';

  return l;
}
#endif
// FIXME this is simply a wrapper for the B-version of the function.
size_t
term_to_str(const struct term_t * const term, size_t * outbuf_size, char * outbuf)
{
  struct buffer_info b =
    {
      .buffer = outbuf,
      .idx = 0,
      .buffer_size = *outbuf_size
    };

  struct buffer_write_result * res = Bterm_to_str(term, &b);
  assert(is_ok_buffer_write_result(res));
  free(res);

  size_t l = b.idx;

  return l;
}

struct buffer_write_result *
Bterm_to_str(const struct term_t * const term, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, term->identifier);
  assert(is_ok_buffer_write_result(res));
  free(res);

  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

#if DEBUG
  char local_buf[BUF_SIZE];
  sprintf(local_buf, "{hash=%d}", hash_term(*term) + 127);
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

#if 0
size_t
predicate_to_str(const struct atom_t * atom, size_t * outbuf_size, char * outbuf)
{
  size_t l = my_strcpy(outbuf, atom->predicate, outbuf_size); // FIXME add error checking

#if DEBUG
  sprintf(&(outbuf[l]), "{hash=%d}", hash_str(atom->predicate) + 127);
  *outbuf_size -= strlen(&(outbuf[l]));
  l += strlen(&(outbuf[l]));
#endif

  return l;
}
#endif

struct buffer_write_result *
Bpredicate_to_str(const struct atom_t * atom, struct buffer_info * dst)
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

#if 0
size_t
atom_to_str(const struct atom_t * atom, size_t * outbuf_size, char * outbuf)
{
  size_t l = predicate_to_str(atom, outbuf_size, outbuf); // FIXME add error checking

  // There needs to be space to store at least "()\0".
  if (*outbuf_size < 3) {
    // FIXME complain
  }

  outbuf[(*outbuf_size)--, l++] = '(';

  for (int i = 0; i < atom->arity; i++) {
    size_t l_sub = term_to_str(&(atom->args[i]), outbuf_size, outbuf + l); // FIXME add error checking

    l += l_sub;

    if (i != atom->arity - 1) {
      // There needs to be space to store at least ", x)\0".
      if (*outbuf_size < 5) {
        // FIXME complain
      }

      // FIXME batch these.
      outbuf[(*outbuf_size)--, l++] = ',';
      outbuf[(*outbuf_size)--, l++] = ' ';
    }
  }

  // FIXME batch these.
  outbuf[(*outbuf_size)--, l++] = ')';

#if DEBUG
  sprintf(&(outbuf[l]), "{hash=%d}", hash_atom(*atom) + 127);
  *outbuf_size -= strlen(&(outbuf[l]));
  l += strlen(&(outbuf[l]));
#endif

  outbuf[(*outbuf_size)--, l++] = '\0';

  return l;
}
#endif

struct buffer_write_result *
Batom_to_str(const struct atom_t * const atom, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = Bpredicate_to_str(atom, dst);
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
    res = Bterm_to_str(&(atom->args[i]), dst);
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
  sprintf(local_buf, "{hash=%d}", hash_atom(*atom) + 127);
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

#if 0
size_t
clause_to_str(const struct clause_t * clause, size_t * outbuf_size, char * outbuf)
{
  size_t l = atom_to_str(&(clause->head), outbuf_size, outbuf); // FIXME add error checking

  (*outbuf_size)++, l--; // chomp the trailing \0.

  if (clause->body_size > 0) {
    // There needs to be space to store at least " :- x().".
    if (*outbuf_size < 8) {
      // FIXME complain
    }

    // FIXME batch these.
    outbuf[(*outbuf_size)--, l++] = ' ';
    outbuf[(*outbuf_size)--, l++] = ':';
    outbuf[(*outbuf_size)--, l++] = '-';
    outbuf[(*outbuf_size)--, l++] = ' ';

    for (int i = 0; i < clause->body_size; i++) {
      size_t l_sub = atom_to_str(&(clause->body[i]), outbuf_size, outbuf + l); // FIXME add error checking

      (*outbuf_size)++, l--; // chomp the trailing \0.

      l += l_sub;

      if (i != clause->body_size - 1) {
        // There needs to be space to store at least ", x().\0".
        if (*outbuf_size < 7) {
          // FIXME complain
        }

        // FIXME batch these.
        outbuf[(*outbuf_size)--, l++] = ',';
        outbuf[(*outbuf_size)--, l++] = ' ';
      }
    }
  }

  // There needs to be space to store at least ".\0".
  if (*outbuf_size < 2) {
    // FIXME complain
  }

  // FIXME batch these.
  outbuf[(*outbuf_size)--, l++] = '.';

#if DEBUG
  sprintf(&(outbuf[l]), "{hash=%d}", hash_clause(*clause) + 127);
  *outbuf_size -= strlen(&(outbuf[l]));
  l += strlen(&(outbuf[l]));
#endif

  outbuf[l] = '\0';

  return l;
}
#endif
// FIXME this is simply a wrapper for the B-version of the function.
size_t
clause_to_str(const struct clause_t * const clause, size_t * outbuf_size, char * outbuf)
{
  struct buffer_info b =
    {
      .buffer = outbuf,
      .idx = 0,
      .buffer_size = *outbuf_size
    };

  struct buffer_write_result * res = Bclause_to_str(clause, &b);
  assert(is_ok_buffer_write_result(res));
  free(res);

  size_t l = b.idx;

  return l;
}

struct buffer_write_result *
Bclause_to_str(const struct clause_t * const clause, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = Batom_to_str(&(clause->head), dst);
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
      res = Batom_to_str(&(clause->body[i]), dst);
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
  sprintf(local_buf, "{hash=%d}", hash_clause(*clause) + 127);
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

#if 0
size_t
program_to_str(const struct program_t * const program, size_t * outbuf_size, char * outbuf)
{
  size_t offset = 0;
  size_t pre_offset;

  for (int i = 0; i < program->no_clauses; i++) {
    pre_offset = clause_to_str(program->program[i], outbuf_size, outbuf + offset); // FIXME add error checking

    offset += pre_offset;

    if (i < program->no_clauses - 1) {
      outbuf[(*outbuf_size)--, offset++] = '\n';
    }
  }

  return offset;
}
#endif
// FIXME this is simply a wrapper for the B-version of the function.
size_t
program_to_str(const struct program_t * const program, size_t * outbuf_size, char * outbuf)
{
  struct buffer_info b =
    {
      .buffer = outbuf,
      .idx = 0,
      .buffer_size = *outbuf_size
    };

  struct buffer_write_result * res = Bprogram_to_str(program, &b);
  assert(is_ok_buffer_write_result(res));
  free(res);

  size_t l = b.idx;

  return l;
}

struct buffer_write_result *
Bprogram_to_str(const struct program_t * const program, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = NULL;

  for (int i = 0; i < program->no_clauses; i++) {
    res = Bclause_to_str(program->program[i], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    if (i < program->no_clauses - 1) {
      unsafe_buffer_char(dst, '\n');
    }
  }

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct term_t *
mk_const(const char * identifier)
{
  char * ident_copy = malloc(sizeof(char) * (strlen(identifier) + 1));
  strcpy(ident_copy, identifier);
  return mk_term(CONST, ident_copy);
}

struct term_t *
mk_var(const char * identifier)
{
  char * ident_copy = malloc(sizeof(char) * (strlen(identifier) + 1));
  strcpy(ident_copy, identifier);
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

DEFINE_LIST_MK(term, term, struct term_t, struct terms_t, /*no const*/)

DEFINE_U8_LIST_LEN(terms)

struct atom_t *
mk_atom(char * predicate, uint8_t arity, const struct terms_t * args) {
  assert(NULL != predicate);

  struct atom_t * at = malloc(sizeof(struct atom_t));
  assert(NULL != at);

  at->predicate = predicate;
  at->arity = arity;

  if (at->arity > 0) {
    at->args = malloc(sizeof(struct term_t) * at->arity);
    for (int i = 0; i < at->arity; i++) {
      at->args[i] = *(args->term);
      args = args->next;
    }
  } else {
    at->args = NULL;
  }

  return at;
}

DEFINE_LIST_MK(atom, atom, struct atom_t, struct atoms_t, /*no const*/)

DEFINE_U8_LIST_LEN(atoms)

struct clause_t *
mk_clause(struct atom_t * head, uint8_t body_size, const struct atoms_t * body) {
  assert(NULL != head);

  struct clause_t * cl = malloc(sizeof(struct clause_t));
  assert(NULL != cl);

  cl->head = *head;
  cl->body_size = body_size;

  if (cl->body_size > 0) {
    cl->body = malloc(sizeof(struct atom_t) * cl->body_size);
    for (int i = 0; i < cl->body_size; i++) {
      cl->body[i] = *(body->atom);
      body = body->next;
    }
  } else {
    cl->body = NULL;
  }

  return cl;
}

DEFINE_LIST_MK(clause, clause, struct clause_t, struct clauses_t, /*no const*/)

DEFINE_U8_LIST_LEN(clauses)

struct program_t *
mk_program(uint8_t no_clauses, const struct clauses_t * program)
{
  struct program_t * p = malloc(sizeof(struct program_t));
  assert(NULL != p);

  p->no_clauses = no_clauses;

  if (no_clauses > 0) {
    p->program = malloc(sizeof(struct clause_t *) * no_clauses);
    for (int i = 0; i < p->no_clauses; i++) {
      p->program[i] = program->clause;
      program = program->next;
    }
  } else {
    p->program = NULL;
  }

  return p;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_term(struct term_t term)
{
  assert(NULL != term.identifier);

  free((void *)term.identifier);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_terms(struct terms_t * terms)
{
  assert(NULL != terms);

  assert(NULL != terms->term);
  free_term(*(terms->term));
  free((void *)terms->term);
  if (NULL != terms->next) {
    free_terms((void *)terms->next);
  }
  free(terms);
}
#pragma GCC diagnostic pop

void
free_atom(struct atom_t at)
{
  assert(NULL != at.predicate);

  DBG("Freeing atom: ");
  DBG_SYNTAX((void *)&at, (x_to_str_t)Batom_to_str);
  DBG("\n");

  free(at.predicate);
  for (int i = 0; i < at.arity; i++) {
    free_term(at.args[i]);
  }
  if (at.arity > 0) {
    // Since we allocated the space for all arguments, rather than for each argument,
    // we deallocate it as such.
    free(at.args);
  }

  // NOTE since we are passed an atom value rather than a pointer to an atom, we
  // don't deallocate the atom -- it's up to a caller to work out if it wants to
  // do that.
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_atoms(struct atoms_t * atoms)
{
  assert(NULL != atoms);

  assert(NULL != atoms->atom);
  free_atom(*(atoms->atom));
  free((void *)atoms->atom);
  if (NULL != atoms->next) {
    free_atoms((void *)atoms->next);
  }
  free(atoms);
}
#pragma GCC diagnostic pop

void
free_clause(struct clause_t clause)
{
  // No need to free clause->head since that's freed when we free this clause's
  // memory.

  assert((0 == clause.body_size && NULL == clause.body) ||
         (clause.body_size > 0 && NULL != clause.body));
  for (int i = 0; i < clause.body_size; i++) {
    free_atom(clause.body[i]);
  }

  if (clause.body_size > 0) {
    // As with terms, we dellocate the whole body at one go, rather than one clause at a time.
    free(clause.body);
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_clauses(struct clauses_t * clauses)
{
  assert(NULL != clauses);

  assert(NULL != clauses->clause);
  free_clause(*(clauses->clause));
  free((void *)clauses->clause);
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
    DBG_SYNTAX((void *)program->program[i], (x_to_str_t)Bclause_to_str);
    DBG("\n");

    assert(NULL != (program->program[i]));
    free_clause(*(program->program[i])); // Free clause contents.
    free((void *)program->program[i]); // Free the clause itsenf.
  }

  if (program->no_clauses > 0) {
    free(program->program); // Free the array of pointers to clauses.
  }

  free(program); // Free the program struct.
}
#pragma GCC diagnostic pop

// FIXME could allocate memory at start to amortise.
void
debug_out_syntax(void * x, int (*x_to_str)(void *, size_t * outbuf_size, char * outbuf))
{
  size_t buf_size = BUF_SIZE;
  char * outbuf = malloc(buf_size);
  assert(NULL != outbuf);
  x_to_str(x, &buf_size, outbuf);
  DBG("%s", outbuf);
  free(outbuf);
}

void
Bdebug_out_syntax(void * x, struct buffer_write_result * (*x_to_str)(void *, struct buffer_info * dst))
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
hash_term(struct term_t term)
{
  char result = hash_str(term.identifier);
  char * cursor;

  for (cursor = (char *)&term.kind; cursor < (char *)&term.kind + sizeof(term_kind_t); cursor ++) {
    result ^= *cursor;
  }

  return result;
}

char
hash_atom(struct atom_t atom)
{
  char result = hash_str(atom.predicate);

  for (int i = 0; i < atom.arity; i++) {
    result = (char)((result * hash_term(atom.args[i])) % 256) - 128;
  }

  return result;
}

char
hash_clause(struct clause_t clause) {
  char result = hash_atom(clause.head);

  for (int i = 0; i < clause.body_size; i++) {
    result ^= ((i + hash_atom(clause.body[i])) % 256) - 128;
  }

  return result;
}

bool
eq_term(const struct term_t * const t1, const struct term_t * const t2, eq_term_error_t * error_code, bool * result)
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

  if (same_kind && !same_identifier) {
    successful = false;
    *error_code = DIFF_IDENTIFIER_SAME_KIND;
  } else if (!same_kind && same_identifier) {
    successful = false;
    *error_code = DIFF_KIND_SAME_IDENTIFIER;
  } else {
    successful = true;
    *error_code = NO_ERROR;
    assert (same_kind == same_identifier);
    *result = same_kind;
  }

  return successful;
}

void
test_clause(void) {
  printf("***test_clause***\n");
  struct clause_t * cl = malloc(sizeof(struct clause_t));
  struct atom_t * at = malloc(sizeof(struct atom_t));
  struct term_t * t = malloc(sizeof(struct term_t));
  char * okK = "ok";
  char * t_ident = malloc(sizeof(char) * (strlen(okK) + 1));
  strcpy(t_ident, okK);
  *t = (struct term_t){.kind = CONST, .identifier = t_ident};

  char * worldK = "world";
  at->predicate = malloc(sizeof(char) * (strlen(worldK) + 1));
  strcpy(at->predicate, worldK);
  at->arity = 1;
  at->args = t;

  char * helloK = "hello";
  cl->head.predicate = malloc(sizeof(char) * (strlen(helloK) + 1));
  strcpy(cl->head.predicate, helloK);
  cl->head.arity = 0;
  cl->head.args = NULL;
  cl->body_size = 1;
  cl->body = at;

#if 0
  size_t remaining_buf_size = BUF_SIZE;
  char * buf = malloc(remaining_buf_size);
  size_t l = clause_to_str(cl, &remaining_buf_size, buf);
  printf("test clause (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);
  free(buf);
#endif

  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = Bclause_to_str(cl, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test clause (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  free_buffer(outbuf);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);

  free_clause(*cl);
  free(cl);
}

struct term_t *
copy_term(const struct term_t * const term)
{
  char * ident_copy = malloc(sizeof(char) * (strlen(term->identifier) + 1));
  strcpy(ident_copy, term->identifier);
  return mk_term(term->kind, ident_copy);
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
    eq_term_error_t error_code;
    while (NULL != cursor) {
      if (eq_term(cursor->term, ss->term, &error_code, &found)) {
        if (found) {
          break;
        }
      } else {
        // FIXME handle error
      }
      cursor = cursor->next;
    }

    if (!found) {
#if DEBUG
#if 0
      size_t remaining_buf_size = BUF_SIZE;
      char * buf = malloc(remaining_buf_size);
      size_t l = term_to_str(ss->term, &remaining_buf_size, buf);
      printf("unsubsumed (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);
      free(buf);
#endif
      struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
      struct buffer_write_result * res = Bterm_to_str(ss->term, dst);
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

#if 0
size_t
terms_to_str(const struct terms_t * const terms, size_t * outbuf_size, char * outbuf)
{
  const struct terms_t * cursor = terms;
  size_t l = 0;
  while (NULL != cursor) {
    l += term_to_str(cursor->term, outbuf_size, outbuf + l); // FIXME add error checking

    if (NULL != terms->next) {
      outbuf[(*outbuf_size)--, l++] = ',';
      outbuf[(*outbuf_size)--, l++] = ' ';
    }

    cursor = cursor->next;
  }

  outbuf[l] = '\0';

  return l;
}
#endif

struct buffer_write_result *
Bterms_to_str(const struct terms_t * const terms, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  const struct terms_t * cursor = terms;

  struct buffer_write_result * res = NULL;

  while (NULL != cursor) {
    res = Bterm_to_str(cursor->term, dst);
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
