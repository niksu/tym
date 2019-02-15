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


This file: String representation.
*/

#ifndef TYM_STRING_IDX_H
#define TYM_STRING_IDX_H

#include <stdbool.h>
#include <stdlib.h>

// TYM_STRING_TYPE values:
// 0: C strings
// 1: abstract type (of C strings)
// 2: abstract type of hashconsed C strings
#ifndef TYM_STRING_TYPE
  #define TYM_STRING_TYPE 2
#endif
// If the value of TYM_STRING_TYPE was given externally then it will be checked later.

#if TYM_STRING_TYPE == 0
  typedef char TymStr;

  #define TYM_STR_DUPLICATE(s) \
    tym_encode_str(strdup(tym_decode_str(s)))
  #define TYM_CSTR_DUPLICATE(s) \
    tym_encode_str(strdup(s))
#elif TYM_STRING_TYPE == 1
  struct TymStrIdxStruct;
  typedef struct TymStrIdxStruct TymStr;
  #define TYM_STR_DUPLICATE(s) \
    tym_encode_str(strdup(tym_decode_str(s)))
  #define TYM_CSTR_DUPLICATE(s) \
    tym_encode_str(strdup(s))
#elif TYM_STRING_TYPE == 2
  struct TymStrHashIdxStruct;
  typedef struct TymStrHashIdxStruct TymStr;
  #define TYM_STR_DUPLICATE(s) s
  #define TYM_CSTR_DUPLICATE(s) tym_encode_str(strdup(s))
  void tym_force_free_str (const struct TymStrHashIdxStruct *);
  void tym_safe_free_str (const struct TymStrHashIdxStruct *);
#else
  #error "Unknown TYM_STRING_TYPE"
#endif

extern const TymStr * TymEmptyString;
extern const TymStr * TymNewLine;

void tym_init_str (void);
void tym_fin_str (void);

const char * tym_decode_str (const TymStr *);
const TymStr * tym_encode_str (const char *);
void tym_free_str (const TymStr *);
size_t tym_len_str (const TymStr *);
int tym_cmp_str (const TymStr *, const TymStr *);
const TymStr * tym_append_str (const TymStr *, const TymStr *);
const TymStr * tym_append_str_destructive (const TymStr * s1, const TymStr * s2);
const TymStr * tym_append_str_destructive1 (const TymStr * s1, const TymStr * s2);
const TymStr * tym_append_str_destructive2 (const TymStr * s1, const TymStr * s2);

void tym_dump_str(void);
extern const bool TymCanDumpStrings;

bool tym_is_special_string(const TymStr * s);

#endif // TYM_STRING_IDX_H
