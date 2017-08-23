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

struct TymStrIdxStruct;
#if 1
typedef char TymStrIdx;
#else
typedef struct TymStrIdxStruct TymStrIdx;
#endif

char * tym_decode_str (TymStrIdx *);
TymStrIdx * tym_encode_str (char *);
void tym_free_str (TymStrIdx *);

#endif // __TYM_STRING_IDX_H__
