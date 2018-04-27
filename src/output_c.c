/*
 * Translating parsed Datalog into C code.
 * Nik Sultana, April 2018.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <time.h>

#include "ast.h"
#include "formula.h"
#include "interface_c.h"
#include "output_c.h"
#include "support.h"

static void
timestamp(char * charbuff, size_t charbuff_len)
{
  time_t now = time(NULL);
  if ((time_t)-1 == now) {
    exit(TYM_TIMESTAMP_ERROR);
  }

  struct tm * now_calendar = localtime(&now);
  if (NULL == now_calendar) {
    exit(TYM_TIMESTAMP_ERROR);
  }

  if (!strftime(charbuff, charbuff_len, "%A %c %z", now_calendar)) {
    exit(TYM_TIMESTAMP_ERROR);
  }
}

void
emit_c_program(struct TymProgram * ParsedInputFileContents,
  struct TymProgram * ParsedQuery)
{
  struct TymSymGen * ng = tym_mk_sym_gen(TYM_CSTR_DUPLICATE("var"));
  const struct TymCSyntax * csyn_program = tym_csyntax_program(ng, ParsedInputFileContents);
  const struct TymCSyntax * csyn_query = tym_csyntax_program(ng, ParsedQuery);

  char charbuff[80/*FIXME const*/];
  timestamp(charbuff, 80/*FIXME const*/);
  printf("// Generated by Tym Datalog version %d.%d at %s\n\n", TYM_VERSION_MAJOR, TYM_VERSION_MINOR, charbuff);
  printf("#include \"tym.h\"\n\n");
  printf("void\n");
  printf("apply (void (*meta_program)(struct TymParams * Params, struct TymProgram * program, struct TymProgram * query), struct TymParams * Params)\n");
  printf("{\n");
  printf("// Program\n%s\n", tym_decode_str(csyn_program->serialised));
  printf("// Query\n%s\n", tym_decode_str(csyn_query->serialised));
  printf("struct TymProgram * program = &%s;\n", tym_decode_str(csyn_program->name));
  printf("struct TymProgram * query = &%s;\n", tym_decode_str(csyn_query->name));
  printf("meta_program(Params, program, query);\n");
  printf("}\n");

  tym_free_sym_gen(ng);
  tym_csyntax_free(csyn_program);
  tym_csyntax_free(csyn_query);
}

