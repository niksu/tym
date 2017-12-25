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

char *
strcpy_prefixed(const char * prefix, const char * str)
{
  char * result = malloc(strlen(prefix) + strlen(str) + 1);
  memcpy(result, prefix, strlen(prefix));
  memcpy(result + strlen(prefix), str, strlen(str));
  result[strlen(prefix) + strlen(str)] = '\0';
  return result;
}
