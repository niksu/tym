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

TYM_HASH_VTYPE
tym_hash_str(TymStr * s)
{
  assert(NULL != s);
  const char * str = tym_decode_str(s);
  assert(NULL != str);

  TYM_HASH_VTYPE result = 0;
  const char * cursor;

  cursor = str;
  while ('\0' != *cursor) {
    result ^= *(cursor++);
  }

  return result;
}
