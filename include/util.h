/*
 * Useful "definitions" of broad scope.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_UTIL_H__
#define __TYM_UTIL_H__

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define BUF_SIZE 1600

#define VERBOSE printf
#define ERR(...) fflush(stdout); fprintf(stderr, __VA_ARGS__)

#if DEBUG
#define DBG fflush(stdout); printf
#else
#define DBG(...)
#endif // DEBUG

#define DECLARE_LIST_TYPE(TYPE_NAME, FIELD_NAME, ELEMENTS_TY) \
  struct TYPE_NAME { \
    struct ELEMENTS_TY * FIELD_NAME; \
    struct TYPE_NAME * next; \
  };
// FIXME include functions for LEN and STR and NEW

#endif /* __TYM_UTIL_H__ */
