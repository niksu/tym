/*
 * Support functions for TYM Datalog.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_SUPPORT_H__
#define __TYM_SUPPORT_H__

#include "ast.h"
#include "buffer.h"
#include "formula.h"
#include "parser.h"
#include "lexer.h"
#include "statement.h"
#include "symbols.h"
#include "translate.h"
#include "util.h"

enum TymFunction {TYM_CONVERT_TO_SMT=0};

struct TymParams {
  char * input_file;
  char verbosity;
  char * query;
  bool test_parsing;
  enum TymFunction function;
};

enum TymReturnCodes {TYM_AOK=0, TYM_UNRECOGNISED_PARAMETER, TYM_NO_INPUT, TYM_INVALID_INPUT};

struct TymProgram * parse(const char * string);
char * read_file(char * filename);
struct TymProgram * tym_parse_input_file(struct TymParams Params);
struct TymProgram * tym_parse_query(struct TymParams Params);
void print_parsed_program(struct TymParams Params, struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);
enum TymReturnCodes process_program(struct TymParams Params, struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);

TYM_DECLARE_LIST_SHALLOW_FREE(stmts, const, struct TymStmts)

#endif // __TYM_SUPPORT_H__
