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

#define HASH_RANGE 256
// NOTE value of TERM_DATABASE_SIZE must be >= the range of the hash function for terms.
#define TERM_DATABASE_SIZE HASH_RANGE

struct term_database_t {
  struct terms_t * herbrand_universe;
  struct terms_t * term_database[TERM_DATABASE_SIZE];
};

struct term_database_t * mk_term_database(void);
bool term_database_add(struct term_t * term, struct term_database_t * tdb);
struct buffer_write_result * term_database_str(struct term_database_t * tdb, struct buffer_info * dst);

struct predicate_t {
  const char * predicate;
  struct clauses_t * bodies;
  uint8_t arity;
};

struct predicate_t * mk_pred(const char * predicate, uint8_t arity);
void free_pred(struct predicate_t * pred);

enum eq_pred_error {NO_ERROR_eq_pred, SAME_PREDICATE_DIFF_ARITY};

bool eq_pred(struct predicate_t p1, struct predicate_t p2, enum eq_pred_error * error_code, bool * result);

DECLARE_MUTABLE_LIST_TYPE(predicates_t, predicate, predicate_t)
DECLARE_MUTABLE_LIST_MK(pred, struct predicate_t, struct predicates_t)

#define ATOM_DATABASE_SIZE HASH_RANGE

struct atom_database_t {
  struct term_database_t * tdb;
  struct predicates_t * atom_database[ATOM_DATABASE_SIZE];
};

struct atom_database_t * mk_atom_database(void);

enum adl_lookup_error {ADL_NO_ERROR = 0, DIFF_ARITY};

bool atom_database_member(const struct atom_t * atom, struct atom_database_t * adb, enum adl_lookup_error * error_code, struct predicate_t ** record);

enum adl_add_error {NO_ATOM_DATABASE = 0};

bool atom_database_add(const struct atom_t * atom, struct atom_database_t * adb, enum adl_add_error * error_code, struct predicate_t ** result);

struct buffer_write_result * atom_database_str(struct atom_database_t * adb, struct buffer_info * dst);
struct predicates_t * atom_database_to_predicates(struct atom_database_t * adb);
struct buffer_write_result * predicate_str(const struct predicate_t * pred, struct buffer_info * dst);

enum cdl_add_error {CDL_ADL_DIFF_ARITY = 0, CDL_ADL_NO_ATOM_DATABASE};

bool clause_database_add(struct clause_t * clause, struct atom_database_t * cdb, enum cdl_add_error *);

size_t num_predicate_bodies (struct predicate_t *);

#endif /* __SYMBOLS_H__ */
