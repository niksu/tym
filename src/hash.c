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


This file: Hashing for TYM Datalog.
*/

#include <assert.h>
#include <stdlib.h>

#include "hash.h"

#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME 0x100000001b3

// NOTE this implements then FNV hash:
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

TYM_HASH_VTYPE
tym_hash_str(const char * str)
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
