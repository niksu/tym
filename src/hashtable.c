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


This file: Hash tabling for TYM Datalog.
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
TYM_DECL_HASHTABLE_CELL(String, const char *, const TymStr *)
TYM_DECL_HASHTABLE(String, const char *, const TymStr *)

static void free_cell(TYM_HASHTABLE_CELL(String) * cell);

static void
free_cell(TYM_HASHTABLE_CELL(String) * cell)
{
  // FIXME This function works correctly, but the the encapsulation
  //       isn't being done right. This function is rather tightly
  //       bound to tym_encode_str, in particular the "v" value
  //       references "k", so we only free one of them.  We call a
  //       "free" function specifically for "struct
  //       TymStrHashIdxStruct *" here to free it as well as
  //       the character string.
  tym_force_free_str(cell->v);
  free((void *)cell);
}

TYM_HASHTABLE(String) *
tym_ht_create(void)
{
  TYM_HASHTABLE(String) * result = malloc(sizeof(*result));
  for (int i = 0; i < TYM_HASH_RANGE; ++i) {
    result->arr[i] = NULL;
  }
  result->free_cell = &free_cell;
  return result;
}

bool
tym_ht_add(TYM_HASHTABLE(String) * ht, const char * key, TYM_HVALUETYPE value)
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
      ht->arr[h]->v = value;
    } else {
      precursor->next = malloc(sizeof(*precursor->next));
      precursor->next->next = NULL;
      precursor->next->k = key;
      precursor->next->v = value;
    }
  }

  return (!exists);
}

TYM_HVALUETYPE
tym_ht_lookup(TYM_HASHTABLE(String) * ht, const char * key)
{
  assert(NULL != ht);
  assert(NULL != key);

  TYM_HVALUETYPE result = NULL;

  TYM_HASH_VTYPE h = tym_hash_str(key);
  TYM_HASHTABLE_CELL(String) * cursor = ht->arr[h];
  while (NULL != cursor) {
    if (0 == strcmp(key, cursor->k)) {
      result = cursor->v;
      break;
    } else {
      cursor = cursor->next;
    }
  }
  return result;
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
      free_cell(ht->arr[i]);
    }
  }
  free(ht);
}

void
tym_ht_dump(TYM_HASHTABLE(String) * ht)
{
  assert(NULL != ht);

  for (int i = 0; i < TYM_HASH_RANGE; i++) {
    TYM_HASHTABLE_CELL(String) * cursor = ht->arr[i];
    while (NULL != cursor) {
      printf("%s : %s\n", cursor->k, tym_decode_str(cursor->v));
      cursor = cursor->next;
    }
  }
}

bool
tym_ht_delete(TYM_HASHTABLE(String) * ht, const char * key)
{
  assert(NULL != ht);
  assert(NULL != key);

  bool result = false;

  TYM_HASH_VTYPE h = tym_hash_str(key);
  TYM_HASHTABLE_CELL(String) * cursor = ht->arr[h];
  TYM_HASHTABLE_CELL(String) * prev_cursor = NULL;
  while (NULL != cursor) {
    if (0 == strcmp(key, cursor->k)) {
      if (cursor == ht->arr[h]) {
        ht->arr[h] = cursor->next;
      } else {
        assert(NULL != prev_cursor);
        prev_cursor->next = cursor->next;
      }
      ht->free_cell(cursor);
      result = true;
      break;
    } else {
      prev_cursor = cursor;
      cursor = cursor->next;
    }
  }

  return result;
}

#endif // TYM_STRING_TYPE == 2
