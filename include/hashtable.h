/*
 * Hash tabling for TYM Datalog.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef TYM_HASHTABLE_H
#define TYM_HASHTABLE_H

#include <stdbool.h>

#include "hash.h"
#include "string_idx.h"

#define TYM_HASH_RANGE 256

#define TYM_HVALUETYPE const TymStr *

#define TYM_HASHTABLE(typename) \
  struct TymHashTable_ ## typename

#define TYM_HASHTABLE_CELL(typename) \
  struct TymHashTableCell_ ## typename

#define TYM_DECL_HASHTABLE_CELL(typename, ktype, vtype) \
  TYM_HASHTABLE_CELL(typename) { \
    ktype k; \
    vtype v; \
    TYM_HASHTABLE_CELL(typename) * next; \
  };

#define TYM_DECL_HASHTABLE(typename, ktype, vtype) \
  TYM_HASHTABLE(typename) { \
    TYM_HASHTABLE_CELL(typename) * arr[TYM_HASH_RANGE]; \
  };

// FIXME currently this only works if TYM_STRING_TYPE == 2.
#if TYM_STRING_TYPE == 2
TYM_HASHTABLE(String);

// FIXME generalise to make parametric to type, beyond "String" and TYM_HVALUETYPE constants
TYM_HASHTABLE(String) * tym_ht_create(void);
bool tym_ht_add(TYM_HASHTABLE(String) *, const char * key, TYM_HVALUETYPE value);
TYM_HVALUETYPE tym_ht_lookup(TYM_HASHTABLE(String) *, const char * key);
// NOTE "delete entry" doesn't seem needed at the moment.
void tym_ht_free(TYM_HASHTABLE(String) *);
#endif // TYM_STRING_TYPE == 2

#endif // TYM_HASHTABLE_H
