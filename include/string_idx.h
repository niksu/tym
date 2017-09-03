/*
 * String representation.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_STRING_IDX_H__
#define __TYM_STRING_IDX_H__

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
#else
  #error "Unknown TYM_STRING_TYPE"
#endif

void tym_init_str (void);
void tym_fin_str (void);

const char * tym_decode_str (const TymStr *);
const TymStr * tym_encode_str (const char *);
void tym_free_str (const TymStr *);
size_t tym_len_str (const TymStr *);
int tym_cmp_str (const TymStr *, const TymStr *);

#endif // __TYM_STRING_IDX_H__
