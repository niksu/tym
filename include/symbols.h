/*
 * Symbol table.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "string_idx.h"
#include "util.h"

#define TYM_HASH_RANGE 256
// NOTE value of TERM_DATABASE_SIZE must be >= the range of the hash function for terms.
#define TYM_TERM_DATABASE_SIZE TYM_HASH_RANGE

struct TymTermDatabase {
  struct TymTerms * herbrand_universe;
  struct TymTerms * term_database[TYM_TERM_DATABASE_SIZE];
};

struct TymTermDatabase * tym_mk_term_database(void);
bool tym_term_database_add(struct TymTerm * term, struct TymTermDatabase * tdb);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_term_database_str(struct TymTermDatabase * tdb, struct TymBufferInfo * dst);

struct TymPredicate {
  str_idx_t predicate;
  struct TymClauses * bodies;
  uint8_t arity;
};

struct TymPredicate * tym_mk_pred(str_idx_t predicate, uint8_t arity);
void tym_free_pred(struct TymPredicate * pred);

enum TymEqPredError {TYM_NO_ERROR_EQ_PRED, SAME_PREDICATE_DIFF_ARITY};

bool tym_eq_pred(struct TymPredicate p1, struct TymPredicate p2, enum TymEqPredError * error_code, bool * result);

TYM_DECLARE_MUTABLE_LIST_TYPE(TymPredicates, predicate, TymPredicate)
TYM_DECLARE_MUTABLE_LIST_MK(pred, struct TymPredicate, struct TymPredicates)

#define TYM_ATOM_DATABASE_SIZE TYM_HASH_RANGE

struct TymAtomDatabase {
  struct TymTermDatabase * tdb;
  struct TymPredicates * atom_database[TYM_ATOM_DATABASE_SIZE];
};

struct TymAtomDatabase * tym_mk_atom_database(void);
void tym_free_atom_database(struct TymAtomDatabase *);

enum TymAdlLookupError {TYM_ADL_NO_ERROR = 0, TYM_DIFF_ARITY};

bool tym_atom_database_member(const struct TymAtom * atom, struct TymAtomDatabase * adb, enum TymAdlLookupError * error_code, struct TymPredicate ** record);

enum TymAdlAddError {TYM_NO_ATOM_DATABASE = 0};

bool tym_atom_database_add(const struct TymAtom * atom, struct TymAtomDatabase * adb, enum TymAdlAddError * error_code, struct TymPredicate ** result);

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_atom_database_str(struct TymAtomDatabase * adb, struct TymBufferInfo * dst);
struct TymPredicates * tym_atom_database_to_predicates(struct TymAtomDatabase * adb);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_predicate_str(const struct TymPredicate * pred, struct TymBufferInfo * dst);

enum TymCdlAddError {TYM_CDL_ADL_DIFF_ARITY = 0, TYM_CDL_ADL_NO_ATOM_DATABASE};

bool tym_clause_database_add(struct TymClause * clause, struct TymAtomDatabase * cdb, enum TymCdlAddError *);

size_t tym_num_predicate_bodies(struct TymPredicate *);

#endif /* __SYMBOLS_H__ */
