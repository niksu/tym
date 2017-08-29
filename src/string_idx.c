/*
 * String representation.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "stdlib.h"
#include "string.h"

#include "string_idx.h"

#if TYM_STRING_TYPE == 0
char *
tym_decode_str (TymStr * s)
{
  return s;
}

TymStr *
tym_encode_str (char * s)
{
  return s;
}

void
tym_free_str (TymStr * s)
{
  free(s);
}

size_t
tym_len_str (const TymStr * s)
{
  return strlen(s);
}

int
tym_cmp_str (const TymStr * s1, const TymStr * s2)
{
  return strcmp(s1, s2);
}
#elif TYM_STRING_TYPE == 1
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

size_t
tym_len_str (const struct TymStrIdxStruct * s)
{
  return strlen(s->content);
}

int
tym_cmp_str (const struct TymStrIdxStruct * s1, const struct TymStrIdxStruct * s2)
{
  return strcmp(s1->content, s2->content);
}
#else
  #error "Unknown TYM_STRING_TYPE"
#endif
