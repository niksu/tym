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

#if 1
typedef char TymStr;
#else
struct TymStrIdxStruct;
typedef struct TymStrIdxStruct TymStr;
#endif

char * tym_decode_str (TymStr *);
TymStr * tym_encode_str (char *);
void tym_free_str (TymStr *);
size_t tym_len_str (const TymStr *);
int tym_cmp_str (const TymStr *, const TymStr *);

#endif // __TYM_STRING_IDX_H__
