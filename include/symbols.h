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
#include "util.h"

#define TYM_HASH_RANGE 256
// NOTE value of TERM_DATABASE_SIZE must be >= the range of the hash function for terms.
#define TYM_TERM_DATABASE_SIZE TYM_HASH_RANGE

struct TymTermDatabase {
  struct TymTerms * herbrand_universe;
  struct TymTerms * term_database[TYM_TERM_DATABASE_SIZE];
};

struct TymTermDatabase * mk_term_database(void);
bool term_database_add(struct TymTerm * term, struct TymTermDatabase * tdb);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * term_database_str(struct TymTermDatabase * tdb, struct TymBufferInfo * dst);

struct TymPredicate {
  const char * predicate;
  struct TymClauses * bodies;
  uint8_t arity;
};

struct TymPredicate * mk_pred(const char * predicate, uint8_t arity);
void free_pred(struct TymPredicate * pred);

enum TymEqPredError {NO_ERROR_eq_pred, SAME_PREDICATE_DIFF_ARITY};

bool eq_pred(struct TymPredicate p1, struct TymPredicate p2, enum TymEqPredError * error_code, bool * result);

TYM_DECLARE_MUTABLE_LIST_TYPE(predicates_t, predicate, TymPredicate)
TYM_DECLARE_MUTABLE_LIST_MK(pred, struct TymPredicate, struct predicates_t)

#define TYM_ATOM_DATABASE_SIZE TYM_HASH_RANGE

struct TymAtomDatabase {
  struct TymTermDatabase * tdb;
  struct predicates_t * atom_database[TYM_ATOM_DATABASE_SIZE];
};

struct TymAtomDatabase * mk_atom_database(void);
void free_atom_database(struct TymAtomDatabase *);

enum adl_lookup_error {ADL_NO_ERROR = 0, DIFF_ARITY};

bool atom_database_member(const struct TymAtom * atom, struct TymAtomDatabase * adb, enum adl_lookup_error * error_code, struct TymPredicate ** record);

enum adl_add_error {NO_ATOM_DATABASE = 0};

bool atom_database_add(const struct TymAtom * atom, struct TymAtomDatabase * adb, enum adl_add_error * error_code, struct TymPredicate ** result);

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * atom_database_str(struct TymAtomDatabase * adb, struct TymBufferInfo * dst);
struct predicates_t * atom_database_to_predicates(struct TymAtomDatabase * adb);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * predicate_str(const struct TymPredicate * pred, struct TymBufferInfo * dst);

enum cdl_add_error {CDL_ADL_DIFF_ARITY = 0, CDL_ADL_NO_ATOM_DATABASE};

bool clause_database_add(struct TymClause * clause, struct TymAtomDatabase * cdb, enum cdl_add_error *);

size_t num_predicate_bodies (struct TymPredicate *);

#endif /* __SYMBOLS_H__ */
