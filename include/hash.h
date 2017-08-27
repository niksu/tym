/*
 * Hashing for TYM Datalog.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_HASH_H__
#define __TYM_HASH_H__

#include "string_idx.h"

#define TYM_HASH_VTYPE uint8_t

TYM_HASH_VTYPE tym_hash_str(TymStr * str);

#endif // __TYM_HASH_H__
