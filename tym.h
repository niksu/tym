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
#include <stdbool.h>
#include <stdio.h>

#define BUF_SIZE 300

#define VERBOSE printf
#define ERR(...) fflush(stdout); fprintf(stderr, __VA_ARGS__)

#if DEBUG
#define DBG fflush(stdout); printf
#else
#define DBG(...)
#endif // DEBUG

struct param_t {
  char * source_file;
  char verbosity;
  char * query;
  bool test_parsing;
};

#endif /* __TYM_H__ */
