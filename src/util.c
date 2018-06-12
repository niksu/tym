/*
 * Useful definitions of broad scope.
 * Nik Sultana, December 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "stdlib.h"
#include "string.h"

#include "util.h"

size_t TYM_BUF_SIZE = 3000;

char *
strcpy_prefixed(const char * prefix, const char * str)
{
  char * result = malloc(strlen(prefix) + strlen(str) + 1);
  memcpy(result, prefix, strlen(prefix));
  memcpy(result + strlen(prefix), str, strlen(str));
  result[strlen(prefix) + strlen(str)] = '\0';
  return result;
}

void
free_const(const void * m)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  free((void *)m);
#pragma GCC diagnostic pop
}
