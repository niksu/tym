/*
 * Symbol table.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

#define HASH_RANGE 256
// NOTE value of TERM_DATABASE_SIZE must be >= the range of the hash function for terms.
#define TERM_DATABASE_SIZE HASH_RANGE

struct term_database_t {
  struct terms_t * herbrand_universe;
  struct terms_t * term_database[TERM_DATABASE_SIZE];
};

struct term_database_t *
mk_term_database(void)
{
  struct term_database_t * result = (struct term_database_t *)malloc(sizeof(struct term_database_t));
  result ->herbrand_universe = NULL;
  for (int i = 0; i < TERM_DATABASE_SIZE; i++) {
    result->term_database[i] = NULL;
  }
  return result;
}

bool
term_database_add(struct term_t * term, struct term_database_t * tdb)
{
  bool exists = false;
  char h = hash_term(*term);
  if (NULL == tdb->term_database[(int)h]) {
    tdb->term_database[(int)h] = mk_term_cell(term, NULL);
    tdb->herbrand_universe = mk_term_cell(term, tdb->herbrand_universe);
  } else {
    struct terms_t * cursor = tdb->term_database[(int)h];
    do {
      bool result;
      eq_term_error_t error_code;
      if (eq_term(*term, *(cursor->term), &error_code, &result)) {
          exists = result;
          break;
      } else {
        // FIXME analyse and and act on error_code.
      }
    } while (NULL != cursor->next);

    if (!exists) {
      cursor->next = mk_term_cell(term, NULL);
      tdb->herbrand_universe = mk_term_cell(term, tdb->herbrand_universe);
    }
  }

  return exists;
}

//struct terms_t *
//term_database_members(void)
//{
//  struct terms_t * result = NULL;
//  for (int i = 0; i < TERM_DATABASE_SIZE; i++) {
//    if (NULL != term_database[i]) {
//      if (NULL == result) {
//        
//      }
//      struct terms_t * cursor = term_database[i];
//      while (NULL != cursor) {
////        struct t
//        cursor = cursor->next;
//      }
//    }
//  }
//  return result;
//}

int
term_database_str(struct term_database_t * tdb, size_t * outbuf_size, char * outbuf)
{
  struct terms_t * cursor = tdb->herbrand_universe;
  int l = 0;

  while (NULL != cursor && *outbuf_size > 0) {
    int l_sub = term_to_str(cursor->term, outbuf_size, outbuf + l);
    if (l_sub < 0) {
      // FIXME complain
      return l_sub;
    }

    l += l_sub;

    outbuf[(*outbuf_size)--, l++] = '\n';
    assert(*outbuf_size > 0);
  }

  return l;
}

struct predicate_t {
  const char * predicate;
  uint8_t arity;
};

struct predicate_t *
mk_pred(const char * predicate, uint8_t arity)
{
  assert(NULL != predicate);

  struct predicate_t * p = (struct predicate_t *)malloc(sizeof(struct predicate_t *));
  assert(NULL != p);

  p->predicate = predicate;
  p->arity = arity;
  return p;
}

struct predicates_t {
  struct predicate_t * predicate;
  struct predicates_t * next;
};

struct predicates_t *
mk_pred_cell(struct predicate_t * pred, struct predicates_t * next)
{
  assert(NULL != pred);

  struct predicates_t * ps = (struct predicates_t *)malloc(sizeof(struct predicates_t *));
  assert(NULL != ps);

  ps->predicate = pred;
  ps->next = next;
  return ps;
}

typedef enum {NO_ERROR_eq_pred, SAME_PREDICATE_DIFF_ARITY} eq_pred_error_t;

bool
eq_pred(struct predicate_t p1, struct predicate_t p2, eq_pred_error_t * error_code, bool * result)
{
  *error_code = NO_ERROR_eq_pred;

  if (&p1 == &p2) {
    *result = true;
    return true;
  }

  if (0 != strcmp(p1.predicate, p2.predicate)) {
    *result = false;
    return true;
  } else {
    if (p1.arity == p2.arity) {
      *result = true;
      return true;
    } else {
      *error_code = SAME_PREDICATE_DIFF_ARITY;
      return false;
    }
  }
}

#define ATOM_DATABASE_SIZE HASH_RANGE

struct atom_database_t {
  struct term_database_t * tdb;
  struct predicates_t * atom_database[ATOM_DATABASE_SIZE];
};

struct atom_database_t *
mk_atom_database(void)
{
  struct atom_database_t * result = (struct atom_database_t *)malloc(sizeof(struct atom_database_t));
  result->tdb = mk_term_database();
  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    result->atom_database[i] = NULL;
  }
  return result;
}

typedef enum {ADL_NO_ERROR, DIFF_ARITY} adl_lookup_error_t;

bool
atom_database_member(struct atom_t * atom, struct atom_database_t * adb, adl_lookup_error_t error_code, bool * result)
{
  bool success = true;
  if (NULL == adb) {
    success = true;
    *result = false;
  } else {
    char h = hash_atom(*atom);

    struct predicate_t * pred = mk_pred(atom->predicate, atom->arity);

    if (NULL == adb->atom_database[(int)h]) {
      success = true;
      *result = false;
      adb->atom_database[(int)h] = mk_pred_cell(pred, NULL);
    } else {
      bool exists = false;

      struct predicates_t * cursor = adb->atom_database[(int)h];
      do {
        bool result;
        eq_pred_error_t error_code;
//        if (eq_atom(*atom, *(cursor->predicate), &error_code, &result)) {
        if (eq_pred(*pred, *(cursor->predicate), &error_code, &result)) {
          exists = result;
          break;
        } else {
          // FIXME analyse and and act on error_code.
          success = false;
        }
      } while (NULL != cursor->next);

      if (success) {
        *result = exists;

        if (!exists) {
          cursor->next = mk_pred_cell(pred, NULL);
        }

        assert(NULL != adb->tdb);
        for (int i = 0; i < atom->arity; i++) {
          (void)term_database_add(&(atom->args[i]), adb->tdb);
        }
      }
    }
  }

  return success;
}

typedef enum {NO_ATOM_DATABASE} adl_add_error_t;

bool
atom_database_add(struct atom_t * atom, struct atom_database_t * adb, adl_add_error_t * error_code, struct predicate_t ** result)
{
  bool success;

  if (NULL == adb) {
    *error_code = NO_ATOM_DATABASE;
    success = false;
  } else {
    char h = hash_atom(*atom);

    struct predicate_t * pred = mk_pred(atom->predicate, atom->arity);

    if (NULL == adb->atom_database[(int)h]) {
      adb->atom_database[(int)h] = mk_pred_cell(pred, NULL);
    } else {
      adb->atom_database[(int)h] = mk_pred_cell(pred, adb->atom_database[(int)h]);
    }

    *result = pred;
    success = true;
  }

  return success;
}

int
atom_database_str(struct atom_database_t * adb, size_t * outbuf_size, char * outbuf)
{
  int l = my_strcpy(outbuf, "Terms:\n", outbuf_size);
  if (l < 0) {
    // FIXME complain
  }
  int l_sub = term_database_str(adb->tdb, outbuf_size, outbuf);
  if (l_sub < 0) {
    // FIXME complain
  }
  l += l_sub;
  outbuf[(*outbuf_size)--, l++] = '\n';

  l_sub = my_strcpy(&outbuf[l], "Predicates:\n", outbuf_size);
  if (l_sub < 0) {
    // FIXME complain
  }
  l += l_sub;

  // FIXME any time we detect that *outbuf_size > 0 then
  //       switch to failure mode.

  struct predicates_t * cursor;

  for (int i = 0; i < ATOM_DATABASE_SIZE; i++) {
    cursor = adb->atom_database[i];
    while (NULL != cursor && *outbuf_size > 0) {
      l_sub = my_strcpy(&outbuf[l], cursor->predicate->predicate, outbuf_size);
      if (l_sub < 0) {
        // FIXME complain
      }
      l += l_sub;

      outbuf[(*outbuf_size)--, l++] = '/';

      l_sub = sprintf(&outbuf[l], "%u", cursor->predicate->arity);
      if (l_sub < 0) {
        // FIXME complain
      }
      l += l_sub;

      outbuf[(*outbuf_size)--, l++] = '\n';

      cursor = cursor->next;
    }
  }

  outbuf[(*outbuf_size)--, l++] = '\n';
  assert(*outbuf_size > 0);

  return l;
}
