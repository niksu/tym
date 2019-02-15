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
    void (*free_cell)(TYM_HASHTABLE_CELL(typename) *); \
  };

// FIXME currently this only works if TYM_STRING_TYPE == 2.
#if TYM_STRING_TYPE == 2
TYM_HASHTABLE(String);

// FIXME generalise to make parametric to type, beyond "String" and TYM_HVALUETYPE constants
TYM_HASHTABLE(String) * tym_ht_create(void);
bool tym_ht_add(TYM_HASHTABLE(String) *, const char * key, TYM_HVALUETYPE value);
TYM_HVALUETYPE tym_ht_lookup(TYM_HASHTABLE(String) *, const char * key);
bool tym_ht_delete(TYM_HASHTABLE(String) *, const char * key);
void tym_ht_dump(TYM_HASHTABLE(String) *);
void tym_ht_free(TYM_HASHTABLE(String) *);
#endif // TYM_STRING_TYPE == 2

#endif // TYM_HASHTABLE_H
