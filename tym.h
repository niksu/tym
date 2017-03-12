/*
 * Useful "definitions" of broad scope.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_H__
#define __TYM_H__

#include <stdarg.h>

#define VERBOSE printf
#define ERR(...) fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
#define DBG printf
#else
#define DBG(...)
#endif // DEBUG

#endif /* __TYM_H__ */
