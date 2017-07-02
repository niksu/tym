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

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "buffer.h"
#include "util.h"

typedef enum {VAR=0, CONST=1, STR=2} TermKind;

struct Term {
  TermKind kind;
  const char * identifier;
};

DECLARE_MUTABLE_LIST_TYPE(Terms, term, Term)
DECLARE_MUTABLE_LIST_MK(term, struct Term, struct Terms)

struct Atom {
  char * predicate;
  uint8_t arity;
  struct Term ** args;
};

DECLARE_MUTABLE_LIST_TYPE(Atoms, atom, Atom)
DECLARE_MUTABLE_LIST_MK(atom, struct Atom, struct Atoms)

struct Clause {
  struct Atom * head;
  uint8_t body_size;
  struct Atom ** body;
};

DECLARE_MUTABLE_LIST_TYPE(Clauses, clause, Clause)
DECLARE_MUTABLE_LIST_MK(clause, struct Clause, struct Clauses)

struct Program {
  uint8_t no_clauses;
  struct Clause ** program;
};

struct buffer_write_result * term_to_str(const struct Term * const term, struct buffer_info * dst);
struct buffer_write_result * terms_to_str(const struct Terms * const terms, struct buffer_info * dst);
struct buffer_write_result * predicate_to_str(const struct Atom * atom, struct buffer_info * dst);
struct buffer_write_result * atom_to_str(const struct Atom * const atom, struct buffer_info * dst);
struct buffer_write_result * clause_to_str(const struct Clause * const clause, struct buffer_info * dst);
struct buffer_write_result * program_to_str(const struct Program * const program, struct buffer_info * dst);

struct Term * mk_const(const char * cp_identifier);
struct Term * mk_var(const char * cp_identifier);
struct Term * mk_term(TermKind kind, const char * identifier);
struct Atom * mk_atom(char * predicate, uint8_t arity, struct Terms * args);
struct Clause * mk_clause(struct Atom * head, uint8_t body_size, struct Atoms * body);
struct Program * mk_program(uint8_t no_clauses, struct Clauses * program);

DECLARE_U8_LIST_LEN(Terms)
DECLARE_U8_LIST_LEN(Atoms)
DECLARE_U8_LIST_LEN(Clauses)

void free_term(struct Term * term);
void free_terms(struct Terms * terms);
void free_atom(struct Atom * atom);
void free_atoms(struct Atoms * atoms);
void free_clause(struct Clause * clause);
void free_clauses(struct Clauses * clauses);
void free_program(struct Program * program);

typedef struct buffer_write_result * (*x_to_str_t)(void *, struct buffer_info * dst);

void debug_out_syntax(void * x, struct buffer_write_result * (*x_to_str)(void *, struct buffer_info * dst));

#if DEBUG
#define DBG_SYNTAX debug_out_syntax
#else
#define DBG_SYNTAX(...)
#endif // DEBUG

char hash_str(const char * str);
char hash_term(const struct Term *);
char hash_atom(const struct Atom *);
char hash_clause(const struct Clause *);

enum eq_term_error {NO_ERROR = 0, DIFF_KIND_SAME_IDENTIFIER};
bool eq_term(const struct Term * const t1, const struct Term * const t2, enum eq_term_error * error_code, bool * result);

struct Term * copy_term(const struct Term * const cp_term);
struct Atom * copy_atom(const struct Atom * const cp_atom);
struct Clause * copy_clause(const struct Clause * const cp_clause);

bool terms_subsumed_by(const struct Terms * const, const struct Terms *);

#endif // __TYM_AST_H__
