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

enum TymTermKind {VAR=0, CONST=1, STR=2};

struct TymTerm {
  enum TymTermKind kind;
  const char * identifier;
};

TYM_DECLARE_MUTABLE_LIST_TYPE(TymTerms, term, TymTerm)
TYM_DECLARE_MUTABLE_LIST_MK(term, struct TymTerm, struct TymTerms)

struct TymAtom {
  char * predicate;
  uint8_t arity;
  struct TymTerm ** args;
};

TYM_DECLARE_MUTABLE_LIST_TYPE(TymAtoms, atom, TymAtom)
TYM_DECLARE_MUTABLE_LIST_MK(atom, struct TymAtom, struct TymAtoms)

struct TymClause {
  struct TymAtom * head;
  uint8_t body_size;
  struct TymAtom ** body;
};

TYM_DECLARE_MUTABLE_LIST_TYPE(TymClauses, clause, TymClause)
TYM_DECLARE_MUTABLE_LIST_MK(clause, struct TymClause, struct TymClauses)

struct TymProgram {
  uint8_t no_clauses;
  struct TymClause ** program;
};

struct buffer_write_result * tym_term_to_str(const struct TymTerm * const term, struct buffer_info * dst);
struct buffer_write_result * tym_terms_to_str(const struct TymTerms * const terms, struct buffer_info * dst);
struct buffer_write_result * tym_predicate_to_str(const struct TymAtom * atom, struct buffer_info * dst);
struct buffer_write_result * tym_atom_to_str(const struct TymAtom * const atom, struct buffer_info * dst);
struct buffer_write_result * tym_clause_to_str(const struct TymClause * const clause, struct buffer_info * dst);
struct buffer_write_result * tym_program_to_str(const struct TymProgram * const program, struct buffer_info * dst);

struct TymTerm * tym_mk_const(const char * cp_identifier);
struct TymTerm * tym_mk_var(const char * cp_identifier);
struct TymTerm * tym_mk_term(enum TymTermKind kind, const char * identifier);
struct TymAtom * tym_mk_atom(char * predicate, uint8_t arity, struct TymTerms * args);
struct TymClause * tym_mk_clause(struct TymAtom * head, uint8_t body_size, struct TymAtoms * body);
struct TymProgram * tym_mk_program(uint8_t no_clauses, struct TymClauses * program);

TYM_DECLARE_U8_LIST_LEN(TymTerms)
TYM_DECLARE_U8_LIST_LEN(TymAtoms)
TYM_DECLARE_U8_LIST_LEN(TymClauses)

void tym_free_term(struct TymTerm * term);
void tym_free_terms(struct TymTerms * terms);
void tym_free_atom(struct TymAtom * atom);
void tym_free_atoms(struct TymAtoms * atoms);
void tym_free_clause(struct TymClause * clause);
void tym_free_clauses(struct TymClauses * clauses);
void tym_free_program(struct TymProgram * program);

typedef struct buffer_write_result * (*tym_x_to_str_t)(void *, struct buffer_info * dst);

void tym_debug_out_syntax(void * x, struct buffer_write_result * (*tym_x_to_str)(void *, struct buffer_info * dst));

#if TYM_DEBUG
#define TYM_DBG_SYNTAX tym_debug_out_syntax
#else
#define TYM_DBG_SYNTAX(...)
#endif // TYM_DEBUG

char tym_hash_str(const char * str);
char tym_hash_term(const struct TymTerm *);
char tym_hash_atom(const struct TymAtom *);
char tym_hash_clause(const struct TymClause *);

enum tym_eq_term_error {NO_ERROR = 0, DIFF_KIND_SAME_IDENTIFIER};
bool tym_eq_term(const struct TymTerm * const t1, const struct TymTerm * const t2, enum tym_eq_term_error * error_code, bool * result);

struct TymTerm * tym_copy_term(const struct TymTerm * const cp_term);
struct TymAtom * tym_copy_atom(const struct TymAtom * const cp_atom);
struct TymClause * tym_copy_clause(const struct TymClause * const cp_clause);

bool tym_terms_subsumed_by(const struct TymTerms * const, const struct TymTerms *);

#endif // __TYM_AST_H__
