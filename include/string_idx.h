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

#if 1
typedef char * str_idx_t;
#endif

char * decode_str (str_idx_t);
str_idx_t encode_str (char *);

#endif // __TYM_STRING_IDX_H__
