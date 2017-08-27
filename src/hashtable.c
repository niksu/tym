/*
 * Hash tabling for TYM Datalog.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "hashtable.h"

// FIXME can factor out hashtable functions from symbols.c
TYM_DECL_HASHTABLE_CELL(String, TymStr *, char *)
TYM_DECL_HASHTABLE(String, TymStr *, char *)

TYM_HASHTABLE(String) *
tym_ht_create(void)
{
  TYM_HASHTABLE(String) * result = malloc(sizeof(*result));
  for (int i = 0; i < TYM_HASH_RANGE; ++i) {
    result->arr[i] = NULL;
  }
  return result;
}

void
tym_ht_add(TYM_HASHTABLE(String) * ht, TymStr * key, TYM_HVALUETYPE * value)
{
  bool exists = false;
  TYM_HASH_VTYPE h = tym_hash_str(key);
  TYM_HASHTABLE_CELL(String) * precursor = NULL;
  TYM_HASHTABLE_CELL(String) * cursor = ht->arr[h];
  while (NULL != cursor) {
    if (0 == tym_cmp_str(key, cursor->k)) {
      exists = true;
      free(*value);
      *value = cursor->v;
      break;
    } else {
      precursor = cursor;
      cursor = cursor->next;
    }
  }

  if (!exists) {
    precursor->next = malloc(sizeof(*precursor->next));
    precursor->next->next = NULL;
    precursor->next->k = key;
    precursor->next->v = *value;
  }
}

TYM_HVALUETYPE
tym_ht_lookup(TYM_HASHTABLE(String) * ht, TymStr * key)
{
  TYM_HASH_VTYPE h = tym_hash_str(key);
  TYM_HASHTABLE_CELL(String) * cursor = ht->arr[h];
  while (NULL != cursor) {
    if (0 == tym_cmp_str(key, cursor->k)) {
      break;
    } else {
      cursor = cursor->next;
    }
  }
  return cursor->v;
}

void
tym_ht_free(TYM_HASHTABLE(String) * ht)
{
  TYM_HASHTABLE_CELL(String) * cursor;
  for (int i = 0; i < TYM_HASH_RANGE; ++i) {
    cursor = ht->arr[i];
    while (NULL != cursor) {
      ht->arr[i] = cursor;
      cursor = cursor->next;
      free(ht->arr[i]);
    }
  }
  free(ht);
}
