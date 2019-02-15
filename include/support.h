/*
Copyright Nik Sultana, 2019

This file is part of TYM Datalog. (https://www.github.com/niksu/tym)

TYM Datalog is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TYM Datalog is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details, a copy of which
is included in the file called LICENSE distributed with TYM Datalog.

You should have received a copy of the GNU Lesser General Public License
along with TYM Datalog.  If not, see <https://www.gnu.org/licenses/>.


This file: Support functions for TYM Datalog.
*/

#ifndef TYM_SUPPORT_H
#define TYM_SUPPORT_H

#include "ast.h"
#include "buffer.h"
#include "buffer_list.h"
#include "formula.h"
#include "parser.h"
#include "lexer.h"
#include "statement.h"
#include "symbols.h"
#include "translate.h"
#include "util.h"

#define TYM_VERSION_MAJOR 1
#define TYM_VERSION_MINOR 0

enum TymFunction {TYM_NOTHING_FUNCTION=0, TYM_TEST_PARSING, TYM_CONVERT_TO_SMT, TYM_CONVERT_TO_SMT_AND_SOLVE, TYM_CONVERT_TO_C, TYM_DUMP_HILBERT_UNIVERSE, TYM_DUMP_ATOMS, TYM_NO_FUNCTION};

enum TymModelOutput {TYM_MODEL_OUTPUT_VALUATION=0, TYM_MODEL_OUTPUT_FACT, TYM_ALL_MODEL_OUTPUT/*Used for testing*/, TYM_NO_MODEL_OUTPUT};

extern enum TymModelOutput TymDefaultModelOutput;
extern const char * const TymDefaultSolverTimeout;

extern const char * TymFunctionCommandMapping[];
extern const char * TymModelOutputCommandMapping[];

const char * tym_functions(void);
const char * tym_model_outputs(void);

extern enum TymSatisfiable TymState_LastSolverResult;

struct TymParams {
  char * input_file;
  char verbosity;
  char * query;
  enum TymFunction function;
  enum TymModelOutput model_output;
  const char * solver_timeout;
};

// NOTE return codes aren't always returned correctly!
//      That is, sometimes Z3 doesn't exit cleanly when
//      it times out. Instead it reports "Error: canceled"
//      and terminates the entire process, and returns a code of 1,
//      overriding TYM's preference to return the value of
//      TYM_SOLVER_GAVEUP
enum TymReturnCode {TYM_AOK=0, TYM_UNRECOGNISED_PARAMETER=1, TYM_NO_INPUT=2, TYM_INVALID_INPUT=3, TYM_SOLVER_GAVEUP=4, TYM_TIMESTAMP_ERROR=5};

struct TymProgram * parse(const char * string);
char * read_file(char * filename);
struct TymProgram * tym_parse_input_file(struct TymParams * Params);
struct TymProgram * tym_parse_query(struct TymParams * Params);
enum TymReturnCode print_parsed_program(struct TymParams * Params, struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);
enum TymReturnCode process_program(struct TymParams * Params, struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);

TYM_DECLARE_LIST_SHALLOW_FREE(stmts, const, struct TymStmts)

#endif // TYM_SUPPORT_H
