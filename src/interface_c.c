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

// FIXME move to ast.h?
const char * TymTermKindStr[] = {"TYM_VAR", "TYM_CONST", "TYM_STR"};

const TymStr *
tym_csyntax_term(const struct TymTerm * term)
{
  const char * identifier = tym_decode_str(term->identifier);
  const char * template = "(struct TymTerm){.kind = ,\
    .identifier = TYM_CSTR_DUPLICATE(\"\")}";
  char * str_buf = malloc(sizeof(*str_buf) *
      (strlen(identifier) + strlen(TymTermKindStr[term->kind]) +
       strlen(template) + 1));
  int buf_occupied = sprintf(str_buf, "(struct TymTerm){.kind = %s,\
    .identifier = TYM_CSTR_DUPLICATE(\"%s\")}",
    TymTermKindStr[term->kind], tym_decode_str(term->identifier));
  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);
  return tym_encode_str(str_buf);
}
