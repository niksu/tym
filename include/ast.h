/*
Copyright Nik Sultana, 2019

This file is part of TYM Datalog. (https://www.github.com/niksu/tym)

TYM Datalog is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TYM Datalog is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details, a copy of which
is included in the file called LICENSE distributed with TYM Datalog.

You should have received a copy of the GNU Lesser General Public License
along with TYM Datalog.  If not, see <https://www.gnu.org/licenses/>.


This file: AST spec.
*/

#ifndef TYM_AST_H
#define TYM_AST_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "buffer.h"
#include "hash.h"
#include "string_idx.h"
#include "util.h"

enum TymSatisfiable {TYM_SAT_NONE=0, // Initial value, before satisfiability check has been attempted.
  TYM_SAT_UNKNOWN, // Used if a backend solver timed out, for example.
  TYM_SAT_YES, TYM_SAT_NO};

enum TymTermKind {TYM_VAR=0, TYM_CONST=1, TYM_STR=2};

struct TymTerm {
  enum TymTermKind kind;
  const TymStr * identifier;
};

TYM_DECLARE_MUTABLE_LIST_TYPE(TymTerms, term, TymTerm)
TYM_DECLARE_MUTABLE_LIST_MK(term, struct TymTerm, struct TymTerms)

struct TymAtom {
  const TymStr * predicate;
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

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_term_str(const struct TymTerm * const term, struct TymBufferInfo * dst);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_terms_str(const struct TymTerms * const terms, struct TymBufferInfo * dst);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_predicate_atom_str(const struct TymAtom * atom, struct TymBufferInfo * dst);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_atom_str(const struct TymAtom * const atom, struct TymBufferInfo * dst);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_clause_str(const struct TymClause * const clause, struct TymBufferInfo * dst);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_program_str(const struct TymProgram * const program, struct TymBufferInfo * dst);

struct TymTerm * tym_mk_term(enum TymTermKind kind, const TymStr * identifier);
struct TymAtom * tym_mk_atom(TymStr * predicate, uint8_t arity, struct TymTerms * args);
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

typedef struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * (*tym_x_str_t)(void *, struct TymBufferInfo * dst);

void tym_debug_out_syntax(void * x, struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * (*tym_x_str)(void *, struct TymBufferInfo * dst));

#if TYM_DEBUG
#define TYM_DBG_SYNTAX tym_debug_out_syntax
#else
#define TYM_DBG_SYNTAX(...)
#endif // TYM_DEBUG

TYM_HASH_VTYPE tym_hash_term(const struct TymTerm *);
TYM_HASH_VTYPE tym_hash_atom(const struct TymAtom *);
TYM_HASH_VTYPE tym_hash_clause(const struct TymClause *);

enum TymEqTermError {TYM_NO_ERROR = 0, TYM_DIFF_KIND_SAME_IDENTIFIER};
bool tym_eq_term(const struct TymTerm * const t1, const struct TymTerm * const t2, enum TymEqTermError * error_code, bool * result);

struct TymTerm * tym_copy_term(const struct TymTerm * const cp_term);
struct TymAtom * tym_copy_atom(const struct TymAtom * const cp_atom);
struct TymClause * tym_copy_clause(const struct TymClause * const cp_clause);

bool tym_terms_subsumed_by(const struct TymTerms * const, const struct TymTerms *);

bool tym_vars_contained(const struct TymTerm *, struct TymTerms *);
struct TymTerms * tym_terms_difference(struct TymTerms *, struct TymTerms *);
void tym_vars_of_atom(struct TymAtom *, struct TymTerms **);
struct TymTerms * tym_hidden_vars_of_clause(const struct TymClause *);

TYM_DECLARE_LIST_SHALLOW_FREE(terms, , struct TymTerms)

struct TymMdlValuation {
  const TymStr * var_name; // Variable identifier chosen by the user.
  const TymStr * const_name; // (Internal) fresh constant identifier chosen by us.
  const TymStr * value;
};

struct TymMdlValuations {
  unsigned count;
  struct TymMdlValuation * v;
};

struct TymMdlValuations * tym_mdl_mk_valuations(const TymStr **, const TymStr **);
void tym_mdl_free_valuations(struct TymMdlValuations *);
void tym_mdl_print_valuations(const struct TymMdlValuations *);
void tym_mdl_reset_valuations(struct TymMdlValuations *);
struct TymProgram * tym_mdl_instantiate_valuation(struct TymProgram * ParsedQuery, struct TymMdlValuations * vals);

#endif // TYM_AST_H
