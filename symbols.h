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
#include "tym.h"

#define HASH_RANGE 256
// NOTE value of TERM_DATABASE_SIZE must be >= the range of the hash function for terms.
#define TERM_DATABASE_SIZE HASH_RANGE

struct term_database_t {
  struct terms_t * herbrand_universe;
  struct terms_t * term_database[TERM_DATABASE_SIZE];
};

struct term_database_t * mk_term_database(void);
bool term_database_add(struct term_t * term, struct term_database_t * tdb);
int term_database_str(struct term_database_t * tdb, size_t * outbuf_size, char * outbuf);

struct predicate_t {
  const char * predicate;
  struct clauses_t * bodies;
  uint8_t arity;
};

struct predicate_t * mk_pred(const char * predicate, uint8_t arity);

typedef enum {NO_ERROR_eq_pred, SAME_PREDICATE_DIFF_ARITY} eq_pred_error_t;

bool eq_pred(struct predicate_t p1, struct predicate_t p2, eq_pred_error_t * error_code, bool * result);

struct predicates_t {
  struct predicate_t * predicate;
  struct predicates_t * next;
};

struct predicates_t * mk_pred_cell(struct predicate_t * pred, struct predicates_t * next);

#define ATOM_DATABASE_SIZE HASH_RANGE

struct atom_database_t {
  struct term_database_t * tdb;
  struct predicates_t * atom_database[ATOM_DATABASE_SIZE];
};

struct atom_database_t * mk_atom_database(void);

typedef enum {ADL_NO_ERROR, DIFF_ARITY} adl_lookup_error_t;

bool atom_database_member(struct atom_t * atom, struct atom_database_t * adb, adl_lookup_error_t * error_code, struct predicate_t ** record);

typedef enum {NO_ATOM_DATABASE} adl_add_error_t;

bool atom_database_add(struct atom_t * atom, struct atom_database_t * adb, adl_add_error_t * error_code, struct predicate_t ** result);

int atom_database_str(struct atom_database_t * adb, size_t * outbuf_size, char * outbuf);
struct predicates_t * atom_database_to_predicates(struct atom_database_t * adb);
int predicate_str(struct predicate_t * pred, size_t * outbuf_size, char * outbuf);

bool clause_database_add(struct clause_t * clause, struct atom_database_t * cdb, void *);

size_t num_predicate_bodies (struct predicate_t *);

#endif /* __SYMBOLS_H__ */
