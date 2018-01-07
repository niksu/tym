/*
 * Translating parsed Datalog into C code.
 * Nik Sultana, November 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "ast.h"
#include "formula.h"
#include "interface_c.h"
#include "module_tests.h"

// FIXME move to ast.h?
const char * TymTermKindStr[] = {"TYM_VAR", "TYM_CONST", "TYM_STR"};

const struct TymCSyntax *
tym_csyntax_term(struct TymSymGen * namegen, const struct TymTerm * term)
{
  struct TymCSyntax * result = malloc(sizeof(*result));

  const char * identifier = tym_decode_str(term->identifier);
  char * str_buf = malloc(sizeof(*str_buf) * TYM_BUF_SIZE);

  result->name = tym_mk_new_var(namegen);
  result->type = TYM_CSTR_DUPLICATE("struct TymTerm");

  int buf_occupied = sprintf(str_buf, "%s %s = (%s){.kind = %s, .identifier = TYM_CSTR_DUPLICATE(\"%s\")};",
    tym_decode_str(result->type), tym_decode_str(result->name),
    tym_decode_str(result->type), TymTermKindStr[term->kind],
    identifier);
  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);

  char * trimmed_str_buf = malloc(sizeof(*trimmed_str_buf) * strlen(str_buf));
  strcpy(trimmed_str_buf, str_buf);
  free(str_buf);
  result->serialised = tym_encode_str(trimmed_str_buf);
  result->kind = TYM_TERM;
  result->original = term;
  return result;
}

/* FIXME copied from ast.c */
void
tym_test_clause_csyn(void) {
  printf("***test_clause_syn***\n");
  struct TymTerm * t = malloc(sizeof *t);
  *t = (struct TymTerm){.kind = TYM_CONST,
    .identifier = TYM_CSTR_DUPLICATE("ok")};

  struct TymSymGen * sg = tym_mk_sym_gen(tym_encode_str(strdup("var")));
  const struct TymCSyntax * csyn = tym_csyntax_term(sg, t);

  printf("serialised: %s\n", tym_decode_str(csyn->serialised));
  const TymStr * malloc_str = tym_csyntax_malloc(csyn);
  printf("serialised: %s\n", tym_decode_str(malloc_str));

  tym_free_sym_gen(sg);
  tym_free_term(t);
  tym_csyntax_free(csyn);
}

const TymStr *
tym_csyntax_malloc(const struct TymCSyntax * csyn)
{
  const TymStr * malloc_str_prefix = TYM_CSTR_DUPLICATE("malloc(sizeof(*");
  const TymStr * malloc_str_suffix = TYM_CSTR_DUPLICATE("))");
  return tym_append_str (malloc_str_prefix, tym_append_str (csyn->name, malloc_str_suffix));
}

void
tym_csyntax_free(const struct TymCSyntax * csyn)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  free((void *)csyn);
#pragma GCC diagnostic pop
}

const struct TymCSyntax *
tym_csyntax_atom(struct TymSymGen * namegen, const struct TymAtom * atom)
{
  struct TymCSyntax * result = malloc(sizeof(*result));
/*
FIXME complete the definition:
* accumulate the definitions of "args", appending them.
* function that takes a list of strings and returns a C array containing those expressions (strings)
*/
  const TymStr * args_identifier = NULL;
  const char * predicate = tym_decode_str(atom->predicate);
  char * str_buf = malloc(sizeof(*str_buf) * TYM_BUF_SIZE);

  result->name = tym_mk_new_var(namegen);
  result->type = TYM_CSTR_DUPLICATE("struct TymAtom");

  int buf_occupied = sprintf(str_buf, "%s %s = (%s){.predicate = TYM_CSTR_DUPLICATE(\"%s\"), .arity = %d, .args = %s};",
    tym_decode_str(result->type), tym_decode_str(result->name),
    tym_decode_str(result->type), predicate, atom->arity,
    tym_decode_str(args_identifier));
  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);

  char * trimmed_str_buf = malloc(sizeof(*trimmed_str_buf) * strlen(str_buf));
  strcpy(trimmed_str_buf, str_buf);
  free(str_buf);
  result->serialised = tym_encode_str(trimmed_str_buf);
  result->kind = TYM_ATOM;
  result->original = atom;
  return result;
}
