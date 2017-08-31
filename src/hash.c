/*
 * Hashing for TYM Datalog.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <assert.h>
#include <stdlib.h>

#include "hash.h"

#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x100000001b3

// NOTE this implements then FNV hash:
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

TYM_HASH_VTYPE
tym_hash_str(char * str)
{
  assert(NULL != str);

  uint64_t result = FNV_OFFSET_BASIS;
  const char * cursor = str;

  while ('\0' != *cursor) {
    result *= FNV_PRIME;
    result ^= (uint64_t)(*cursor);
    cursor++;
  }

  return (TYM_HASH_VTYPE)result;
}
