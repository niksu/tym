/*
 * Hashing for TYM Datalog.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef TYM_HASH_H
#define TYM_HASH_H

#include <stdint.h>

#define TYM_HASH_VTYPE uint8_t

TYM_HASH_VTYPE tym_hash_str(const char * str);

#endif // TYM_HASH_H
