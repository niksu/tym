/*
 * AST supporting functions.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "tym.h"

int
my_strcpy(char * dst, const char * src, size_t * space)
{
  int l = strlen(src);
  if (l < *space) {
    strcpy(dst, src);
  } else {
    return -1;
  }

  *space -= l;
  return l;
}

int
term_to_str(struct term_t * term, size_t * outbuf_size, char * outbuf)
{
  int l = my_strcpy(outbuf, term->identifier, outbuf_size);
  if (l < 0) {
    // FIXME complain
  }

  return l;
}

int
predicate_to_str(struct atom_t * atom, size_t * outbuf_size, char * outbuf)
{
  int l = my_strcpy(outbuf, atom->predicate, outbuf_size);
  if (l < 0) {
    // FIXME complain
  }

  return l;
}

int
atom_to_str(struct atom_t * atom, size_t * outbuf_size, char * outbuf)
{
  int l = predicate_to_str(atom, outbuf_size, outbuf);
  if (l < 0) {
    // FIXME complain
    return l;
  }

  // There needs to be space to store at least "()\0".
  if (*outbuf_size < 3) {
    // FIXME complain
    return -1;
  }

  outbuf[(*outbuf_size)--, l++] = '(';

  for (int i = 0; i < atom->arity; i++) {
    int l_sub = term_to_str(&(atom->args[i]), outbuf_size, outbuf + l);
    if (l_sub < 0) {
      // FIXME complain
      return l_sub;
    }

    l += l_sub;

    if (i != atom->arity - 1) {
      // There needs to be space to store at least ", x)\0".
      if (*outbuf_size < 5) {
        // FIXME complain
        return -1;
      }

      // FIXME batch these.
      outbuf[(*outbuf_size)--, l++] = ',';
      outbuf[(*outbuf_size)--, l++] = ' ';
    }
  }

  // FIXME batch these.
  outbuf[(*outbuf_size)--, l++] = ')';
  outbuf[(*outbuf_size)--, l++] = '\0';

  return l;
}

int
clause_to_str(struct clause_t * clause, size_t * outbuf_size, char * outbuf)
{
  int l = atom_to_str(&(clause->head), outbuf_size, outbuf);
  if (l < 0) {
    // FIXME complain
    return l;
  }

  (*outbuf_size)++, l--; // chomp the trailing \0.

  if (clause->body_size > 0) {
    // There needs to be space to store at least " :- x().".
    if (*outbuf_size < 8) {
      // FIXME complain
      return -1;
    }

    // FIXME batch these.
    outbuf[(*outbuf_size)--, l++] = ' ';
    outbuf[(*outbuf_size)--, l++] = ':';
    outbuf[(*outbuf_size)--, l++] = '-';
    outbuf[(*outbuf_size)--, l++] = ' ';

    for (int i = 0; i < clause->body_size; i++) {
      int l_sub = atom_to_str(&(clause->body[i]), outbuf_size, outbuf + l);

      if (l_sub < 0) {
        // FIXME complain
        return l_sub;
      }

      (*outbuf_size)++, l--; // chomp the trailing \0.

      l += l_sub;

      if (i != clause->body_size - 1) {
        // There needs to be space to store at least ", x().\0".
        if (*outbuf_size < 7) {
          // FIXME complain
          return -1;
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
    return -1;
  }

  // FIXME batch these.
  outbuf[(*outbuf_size)--, l++] = '.';
  outbuf[(*outbuf_size)--, l++] = '\0';

  return l;
}

int
program_to_str(struct program_t * program, size_t * outbuf_size, char * outbuf)
{
  int offset = 0;
  int pre_offset;

  for (int i = 0; i < program->no_clauses; i++) {
    pre_offset = clause_to_str(program->program[i], outbuf_size, outbuf + offset);
    if (pre_offset < 0) {
      // FIXME complain
      return -1;
    }

    offset += pre_offset;

    if (i < program->no_clauses - 1) {
      //(*outbuf_size)++, offset--; // chomp trailing \0
      outbuf[offset - 1] = '\n';
    }
  }

  return offset;
}

struct term_t *
mk_term(term_kind_t kind, char * identifier)
{
  struct term_t * t = (struct term_t *)malloc(sizeof(struct term_t));
  t->kind = kind;
  t->identifier = identifier;
  return t;
}

struct terms_t *
mk_term_cell(struct term_t * term, struct terms_t * next)
{
  struct terms_t * ts = (struct terms_t *)malloc(sizeof(struct terms_t));
  ts->term = term;
  ts->next = next;
  return ts;
}

int
len_term_cell(struct terms_t * next)
{
  int result = 0;

  while (NULL != next) {
    result++;
    next = next->next;
  }

  return result;
}

struct atom_t *
mk_atom(char * predicate, uint8_t arity, struct terms_t * args) {
  struct atom_t * at = (struct atom_t *)malloc(sizeof(struct atom_t));
  at->predicate = predicate;
  at->arity = arity;

  if (at->arity > 0) {
    at->args = (struct term_t *)malloc(sizeof(struct term_t) * at->arity);
    for (int i = 0; i < at->arity; i++) {
      at->args[i] = *(args->term);
      args = args->next;
    }
  }

  return at;
}

struct atoms_t *
mk_atom_cell(struct atom_t * atom, struct atoms_t * next)
{
  struct atoms_t * ats = (struct atoms_t *)malloc(sizeof(struct atoms_t));
  ats->atom = atom;
  ats->next = next;
  return ats;
}

int
len_atom_cell(struct atoms_t * next)
{
  int result = 0;

  while (NULL != next) {
    result++;
    next = next->next;
  }

  return result;
}

struct clause_t *
mk_clause(struct atom_t * head, uint8_t body_size, struct atoms_t * body) {
  struct clause_t * cl = (struct clause_t *)malloc(sizeof(struct clause_t));
  cl->head = *head;
  cl->body_size = body_size;

  if (cl->body_size > 0) {
    cl->body = (struct atom_t *)malloc(sizeof(struct atom_t) * cl->body_size);
    for (int i = 0; i < cl->body_size; i++) {
      cl->body[i] = *(body->atom);
      body = body->next;
    }
  }

  return cl;
}

struct clauses_t *
mk_clause_cell(struct clause_t * clause, struct clauses_t * next)
{
  struct clauses_t * cls = (struct clauses_t *)malloc(sizeof(struct clauses_t));
  cls->clause = clause;
  cls->next = next;
  return cls;
}

int
len_clause_cell(struct clauses_t * next)
{
  int result = 0;

  while (NULL != next) {
    result++;
    next = next->next;
  }

  return result;
}

struct program_t *
mk_program(uint8_t no_clauses, struct clauses_t * program)
{
  struct program_t * p = (struct program_t *)malloc(sizeof(struct program_t));
  p->no_clauses = no_clauses;

  if (no_clauses > 0) {
    p->program = (struct clause_t **)malloc(sizeof(struct clause_t **) * no_clauses);
    for (int i = 0; i < p->no_clauses; i++) {
      p->program[i] = program->clause;
      program = program->next;
    }
  }

  return p;
}

void
free_term(struct term_t term)
{
  free(term.identifier);
}

void
free_terms(struct terms_t * terms)
{
  free_term(*(terms->term));
  free(terms->term);
  if (NULL != terms->next) {
    free_terms(terms->next);
  }
  free(terms);
}

void
free_atom(struct atom_t atom)
{
  DBG("Freeing atom: ");
  DBG_SYNTAX((void *)&atom, (x_to_str_t)atom_to_str);
  DBG("\n");

  free(atom.predicate);
  for (int i = 0; i < atom.arity; i++) {
    free_term(atom.args[i]);
  }
  // Since we allocated the space for all arguments, rather than for each argument,
  // we deallocate it as such.
  free(atom.args);

  // NOTE since we are passed an atom value rather than a pointer to an atom, we
  // don't deallocate the atom -- it's up to a caller to work out if it wants to
  // do that.
}

void
free_atoms(struct atoms_t * atoms)
{
  free_atom(*(atoms->atom));
  free(atoms->atom);
  if (NULL != atoms->next) {
    free_atoms(atoms->next);
  }
  free(atoms);
}

void
free_clause(struct clause_t clause)
{
  // No need to free clause->head since that's freed when we free this clause's
  // memory.
  for (int i = 0; i < clause.body_size; i++) {
    free_atom(clause.body[i]);
  }
  // As with terms, we dellocate the whole body at one go, rather than one clause at a time.
  free(clause.body);
}

void
free_clauses(struct clauses_t * clauses)
{
  free_clause(*(clauses->clause));
  free(clauses->clause);
  if (NULL != clauses->next) {
    free_clauses(clauses->next);
  }
  free(clauses);
}

void
free_program(struct program_t * program)
{
  for (int i = 0; i < program->no_clauses; i++) {
    DBG("Freeing clause %d: ", i);
    DBG_SYNTAX((void *)program->program[i], (x_to_str_t)clause_to_str);
    DBG("\n");

    free_clause(*(program->program[i])); // Free clause contents.
    free(program->program[i]); // Free the clause itsenf.
  }
  free(program->program); // Free the array of pointers to clauses.
  free(program); // Free the program struct.
}

// FIXME could allocate memory at start to amortise.
void
debug_out_syntax(void * x, int (*x_to_str)(void *, size_t * outbuf_size, char * outbuf))
{
  size_t buf_size = BUF_SIZE;
  char * outbuf = (char *)malloc(buf_size);
  x_to_str(x, &buf_size, outbuf);
  DBG("%s", outbuf);
  free(outbuf);
}
