/*
 * Support functions for TYM Datalog.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef TYM_SUPPORT_H
#define TYM_SUPPORT_H

#include "ast.h"
#include "buffer.h"
#include "formula.h"
#include "parser.h"
#include "lexer.h"
#include "statement.h"
#include "symbols.h"
#include "translate.h"
#include "util.h"

enum TymFunction {TYM_TEST_PARSING=0, TYM_CONVERT_TO_SMT, TYM_CONVERT_TO_SMT_AND_SOLVE, TYM_NO_FUNCTION};

enum TymModelOutput {TYM_MODEL_OUTPUT_VALUATION=0, TYM_MODEL_OUTPUT_FACT, TYM_ALL_MODEL_OUTPUT/*Used for testing*/, TYM_NO_MODEL_OUTPUT};

extern enum TymModelOutput TymDefaultModelOutput;

extern const char * TymFunctionCommandMapping[];
extern const char * TymModelOutputCommandMapping[];

const char * tym_functions(void);
const char * tym_model_outputs(void);

struct TymParams {
  char * input_file;
  char verbosity;
  char * query;
  enum TymFunction function;
  enum TymModelOutput model_output;
};

enum TymReturnCodes {TYM_AOK=0, TYM_UNRECOGNISED_PARAMETER, TYM_NO_INPUT, TYM_INVALID_INPUT};

struct TymProgram * parse(const char * string);
char * read_file(char * filename);
struct TymProgram * tym_parse_input_file(struct TymParams Params);
struct TymProgram * tym_parse_query(struct TymParams Params);
void print_parsed_program(struct TymParams Params, struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);
enum TymReturnCodes process_program(struct TymParams Params, struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);

TYM_DECLARE_LIST_SHALLOW_FREE(stmts, const, struct TymStmts)

#endif // TYM_SUPPORT_H
