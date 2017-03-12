/*
 * AST spec.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_AST_H__
#define __TYM_AST_H__

#include <stdlib.h>
#include <stdint.h>

typedef enum {Var, Const, Str} term_kind_t;

struct term_t {
  term_kind_t kind;
  char * identifier;
};

struct terms_t {
  struct term_t * term;
  struct terms_t * next;
};

struct atom_t {
  char * predicate;
  uint8_t arity;
  struct term_t * args;
};

struct atoms_t {
  struct atom_t * atom;
  struct atoms_t * next;
};

struct clause_t {
  struct atom_t head;
  uint8_t body_size;
  struct atom_t * body;
};

struct clauses_t {
  struct clause_t * clause;
  struct clauses_t * next;
};

struct program_t {
  uint8_t no_clauses;
  struct clause_t ** program;
};

int my_strcpy(char * dst, const char * src, size_t * space);
int atom_to_str(struct atom_t * atom, size_t * outbuf_size, char * outbuf);
int clause_to_str(struct clause_t * clause, size_t * outbuf_size, char * outbuf);
int program_to_str(struct program_t * clause, size_t * outbuf_size, char * outbuf);

struct term_t * mk_term(term_kind_t kind, char * identifier);
struct atom_t * mk_atom(char * predicate, uint8_t arity, struct terms_t * args);
struct clause_t * mk_clause(struct atom_t * head, uint8_t body_size, struct atoms_t * body);
struct program_t * mk_program(uint8_t no_clauses, struct clauses_t * program);

struct terms_t * mk_term_cell(struct term_t * term, struct terms_t * next);
struct atoms_t * mk_atom_cell(struct atom_t * term, struct atoms_t * next);
struct clauses_t * mk_clause_cell(struct clause_t * term, struct clauses_t * next);

int len_term_cell(struct terms_t * next);
int len_atom_cell(struct atoms_t * next);
int len_clause_cell(struct clauses_t * next);

#endif // __TYM_AST_H__
