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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

// FIXME currently this only works if TYM_STRING_TYPE == 2.
#if TYM_STRING_TYPE == 2
// FIXME can factor out hashtable functions from symbols.c
TYM_DECL_HASHTABLE_CELL(String, char *, TymStr *)
TYM_DECL_HASHTABLE(String, char *, TymStr *)

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
tym_ht_add(TYM_HASHTABLE(String) * ht, char * key, TYM_HVALUETYPE * value)
{
  assert(NULL != ht);
  assert(NULL != key);
  assert(NULL != value);

  bool exists = false;
  TYM_HASH_VTYPE h = tym_hash_str(key);
  TYM_HASHTABLE_CELL(String) * precursor = NULL;
  TYM_HASHTABLE_CELL(String) * cursor = ht->arr[h];
  while (NULL != cursor) {
    if (0 == strcmp(key, cursor->k)) {
      exists = true;
      tym_force_free_str(*value);
      *value = cursor->v;
      break;
    } else {
      precursor = cursor;
      cursor = cursor->next;
    }
  }

  if (!exists) {
    if (NULL == precursor) {
      // This is the first element we're adding to this bucket.
      ht->arr[h] = malloc(sizeof(*precursor));
      ht->arr[h]->next = NULL;
      ht->arr[h]->k = key;
      ht->arr[h]->v = *value;
    } else {
      precursor->next = malloc(sizeof(*precursor->next));
      precursor->next->next = NULL;
      precursor->next->k = key;
      precursor->next->v = *value;
    }
  }
}

TYM_HVALUETYPE
tym_ht_lookup(TYM_HASHTABLE(String) * ht, char * key)
{
  assert(NULL != ht);
  assert(NULL != key);

  TYM_HASH_VTYPE h = tym_hash_str(key);
  TYM_HASHTABLE_CELL(String) * cursor = ht->arr[h];
  while (NULL != cursor) {
    if (0 == strcmp(key, cursor->k)) {
      return cursor->v;
    } else {
      cursor = cursor->next;
    }
  }
  return NULL;
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
      // NOTE "free(ht->arr[i]->k);" is done implicitly when ->v is freed below,
      //      since the value contains the key as content.
      tym_force_free_str(ht->arr[i]->v);
      free(ht->arr[i]);
    }
  }
  free(ht);
}

#endif // TYM_STRING_TYPE == 2
