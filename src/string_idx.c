/*
 * String representation.
 * Nik Sultana, August 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "string_idx.h"

#if TYM_STRING_TYPE == 0
void
tym_init_str(void)
{
  // Nothing needed here.
}

void
tym_fin_str(void)
{
  // Nothing needed here.
}

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

void
tym_init_str(void)
{
  // Nothing needed here.
}

void
tym_fin_str(void)
{
  // Nothing needed here.
}

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
#elif TYM_STRING_TYPE == 2
#include "hashtable.h"

TYM_HASHTABLE(String) * stringhash = NULL;
struct TymStrHashIdxStruct {
  char * content;
};

void
tym_init_str(void)
{
  assert(NULL == stringhash);
  stringhash = tym_ht_create();
}

void
tym_fin_str(void)
{
  assert(NULL != stringhash);
  tym_ht_free(stringhash);
  stringhash = NULL;
}

struct TymStrHashIdxStruct *
tym_encode_str (char * s)
{
  assert(NULL != stringhash);

  struct TymStrHashIdxStruct * result = malloc(sizeof(*result));
  result->content = s;
  tym_ht_add(stringhash, s, &result);
  return result;
}

char *
tym_decode_str (struct TymStrHashIdxStruct * s)
{
  assert(NULL != stringhash);

  return s->content;
}

void
tym_free_str (struct TymStrHashIdxStruct * s)
{
  assert(NULL != stringhash);

  assert(NULL != s);

  // Do nothing -- everything will be freed when "fin" is called.
  // NOTE for actual freeing see "tym_force_free_str"
}

void
tym_force_free_str (struct TymStrHashIdxStruct * s)
{
  assert(NULL != stringhash);

  assert(NULL != s);
  assert(NULL != s->content);

  free(s->content);
  free(s);
}

size_t
tym_len_str (const struct TymStrHashIdxStruct * s)
{
  assert(NULL != stringhash);

  return strlen(s->content);
}

int
tym_cmp_str (const struct TymStrHashIdxStruct * s1, const struct TymStrHashIdxStruct * s2)
{
  assert(NULL != stringhash);

  return strcmp(s1->content, s2->content);
}
#else
  #error "Unknown TYM_STRING_TYPE"
#endif
