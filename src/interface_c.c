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
  const char * template = "(struct TymTerm){.kind = ,\
    .identifier = TYM_CSTR_DUPLICATE(\"\")}";
  char * str_buf = malloc(sizeof(*str_buf) *
      (strlen(identifier) + strlen(TymTermKindStr[term->kind]) +
       strlen(template) + 1));
  int buf_occupied = sprintf(str_buf, "(struct TymTerm){.kind = %s, .identifier = TYM_CSTR_DUPLICATE(\"%s\")}",
    TymTermKindStr[term->kind], tym_decode_str(term->identifier));
  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);

  result->type = TYM_CSTR_DUPLICATE("struct TymTerm");
  result->name = tym_mk_new_var(namegen);
  result->definition = tym_encode_str(str_buf);
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

  printf("serialised: %s %s = %s\n", tym_decode_str(csyn->type), tym_decode_str(csyn->name), tym_decode_str(csyn->definition));

  tym_free_sym_gen(sg);
  tym_free_term(t);
  tym_csyntax_free(csyn);
}

void
tym_csyntax_free(const struct TymCSyntax * csyn)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  free((void *)csyn);
#pragma GCC diagnostic pop
}
