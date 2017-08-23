/*
 * String representation.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "stdlib.h"

#include "string_idx.h"

#if 1
char *
tym_decode_str (TymStrIdx * s)
{
  return s;
}

TymStrIdx *
tym_encode_str (char * s)
{
  return s;
}

void
tym_free_str (TymStrIdx * s)
{
  free(s);
}
#else
struct TymStrIdxStruct {
  char * content;
};

char *
tym_decode_str (struct TymStrIdxStruct * s)
{
  return s->content;
}

struct TymStrIdxStruct *
tym_encode_str (char * s)
{
  struct TymStrIdxStruct * result = malloc(sizeof(*result));
  result->content = s;
  return result;
}

void
tym_free_str (struct TymStrIdxStruct * s)
{
  free(s->content);
  free(s);
}
#endif
