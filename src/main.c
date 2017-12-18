/*
 * TYM Datalog.
 * Nik Sultana, March 2017.
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tym.h"
#include "module_tests.h"

static void show_usage(const char * const);

static void
show_usage(const char * const argv_0)
{
  // FIXME include name + version + url
  // FIXME include description of each parameter
  printf("usage: %s PARAMETERS \n"
         " Mandatory parameters: \n"
         "   -i, --input_file FILENAME \n"
         "   -f, --function FUNCTION (%s)\n"
         " Optional parameters: \n"
         "   -q, --query QUERY \n"
         "   -m, --model_output MODEL_OUTPUT (%s). Default: %s\n"
         "   -v, --verbose \n"
         "   --max_var_width N \n"
         "   --solver_timeout N (in milliseconds). Default: %s\n"
         "   -h \n", argv_0, tym_functions(), tym_model_outputs(),
         TymModelOutputCommandMapping[TymDefaultModelOutput],
        TymDefaultSolverTimeout);
}

int
main(int argc, char ** argv)
{
#ifdef TYM_TESTING
  tym_init_str();
  tym_test_clause();
  tym_test_formula();
  tym_test_statement();
  tym_fin_str();
  exit(0);
#endif

  struct TymParams Params = {
    .input_file = NULL,
    .verbosity = 0,
    .query = NULL,
    .function = TYM_NO_FUNCTION,
    .model_output = TymDefaultModelOutput,
    .solver_timeout = TymDefaultSolverTimeout
  };

  static struct option long_options[] = {
#define LONG_OPT_INPUT 1
    {"input_file", required_argument, NULL, LONG_OPT_INPUT},
#define LONG_OPT_VERBOSE 2
    {"verbose", no_argument, NULL, LONG_OPT_VERBOSE},
#define LONG_OPT_QUERY 3
    {"query", required_argument, NULL, LONG_OPT_QUERY},
#define LONG_OPT_MAX_VAR_WIDTH 4
    {"max_var_width", required_argument, NULL, LONG_OPT_MAX_VAR_WIDTH},
#define LONG_OPT_FUNCTION 5
    {"function", required_argument, NULL, LONG_OPT_FUNCTION},
#define LONG_OPT_MODEL_OUTPUT 6
    {"model_output", required_argument, NULL, LONG_OPT_MODEL_OUTPUT},
#define LONG_OPT_SOLVER_TIMEOUT 7
    {"solver_timeout", required_argument, NULL, LONG_OPT_SOLVER_TIMEOUT}
  };

  int option_index = 0;
  long v;

  // FIXME: "-f smt_output ... -m fact" doesn't make sense, but we don't emit a warning.
  int option;
  while ((option = getopt_long(argc, argv, "f:hi:m:q:v", long_options,
          &option_index)) != -1) {
    switch (option) {
    case LONG_OPT_INPUT:
    case 'i':
      Params.input_file = malloc(strlen(optarg) + 1);
      strcpy(Params.input_file, optarg);
      break;
    case LONG_OPT_FUNCTION:
    case 'f':
      Params.function = TYM_NO_FUNCTION;
      for (unsigned i = 0; i < TYM_NO_FUNCTION; ++i) {
         if (0 == strcmp(optarg, TymFunctionCommandMapping[i])) {
            Params.function = i;
         }
      }
      if (TYM_NO_FUNCTION == Params.function) {
        TYM_ERR("Unrecognized function: %s\n", optarg);
        return TYM_UNRECOGNISED_PARAMETER;
      }
      break;
    case LONG_OPT_MODEL_OUTPUT:
    case 'm':
      Params.model_output = TYM_NO_MODEL_OUTPUT;
      for (unsigned i = 0; i < TYM_NO_MODEL_OUTPUT; ++i) {
         if (0 == strcmp(optarg, TymModelOutputCommandMapping[i])) {
            Params.model_output = i;
            break;
         }
      }
      if (TYM_NO_MODEL_OUTPUT == Params.model_output) {
        TYM_ERR("Unrecognized model-output: %s\n", optarg);
        return TYM_UNRECOGNISED_PARAMETER;
      }
      break;
    case LONG_OPT_VERBOSE:
    case 'v':
      Params.verbosity = 1;
      break;
    case LONG_OPT_QUERY:
    case 'q':
      Params.query = malloc(strlen(optarg) + 1);
      strcpy(Params.query, optarg);
      break;
    case LONG_OPT_MAX_VAR_WIDTH:
      v = strtol(optarg, NULL, 10);
      assert(v <= UINT8_MAX);
      TymMaxVarWidth = (uint8_t)v;
      break;
    case LONG_OPT_SOLVER_TIMEOUT:
      Params.solver_timeout = strdup(optarg);
      break;
    case 'h':
      show_usage(argv[0]);
      return TYM_AOK;
    default:
      TYM_ERR("Terminating on unrecognized option\n"); // The offending option would have been reported by getopt by this point.
      show_usage(argv[0]);
      return TYM_UNRECOGNISED_PARAMETER;
    }
  }

  if (optind != argc) {
    TYM_ERR("Unrecognised parameter%s:", optind + 1 < argc ? "s" : "");
    for (int i = optind; i < argc; ++i) {
      TYM_ERR(" %s", argv[i]);
    }
    TYM_ERR("\n");
    return TYM_UNRECOGNISED_PARAMETER;
  }

  if (Params.verbosity > 0) {
#ifdef TYM_DEBUG
    TYM_VERBOSE("TYM_DEBUG = %d\n", TYM_DEBUG);
#else
    TYM_VERBOSE("TYM_DEBUG Undefined\n");
#endif
    TYM_VERBOSE("TYM_STRING_TYPE = %d\n", TYM_STRING_TYPE); // TYM_STRING_TYPE macro should always be defined.

#ifdef TYM_TESTING
    // If TYM_TESTING is defined then this code should not be reachable.
    assert(TYM_TESTING != TYM_TESTING);
#endif
    TYM_VERBOSE("TYM_TESTING Undefined\n");

#ifdef TYM_INTERFACE_Z3
    TYM_VERBOSE("TYM_INTERFACE_Z3 = %d\n", TYM_INTERFACE_Z3);
#else
    TYM_VERBOSE("TYM_INTERFACE_Z3 Undefined\n");
#endif

    TYM_VERBOSE("input_fine = %s\n", Params.input_file);
    TYM_VERBOSE("verbosity = %d\n", Params.verbosity);
    TYM_VERBOSE("query = %s\n", Params.query);
    TYM_VERBOSE("function = %s\n", TymFunctionCommandMapping[Params.function]);
    TYM_VERBOSE("model_output = %s\n", TymModelOutputCommandMapping[Params.model_output]);
    TYM_VERBOSE("solver_timeout = %s\n", Params.solver_timeout);
  }

  assert(Params.function != TYM_NO_FUNCTION);

#ifndef TYM_INTERFACE_Z3
    if (TYM_CONVERT_TO_SMT_AND_SOLVE == Params.function) {
      TYM_ERR("Must define TYM_INTERFACE_Z3 at build time in order to use function '%s'\n", TymFunctionCommandMapping[TYM_CONVERT_TO_SMT_AND_SOLVE]);
      return TYM_UNRECOGNISED_PARAMETER;
    }
#endif // TYM_INTERFACE_Z3

  tym_init_str();

  enum TymReturnCodes result = TYM_AOK;

  struct TymProgram * ParsedInputFileContents = tym_parse_input_file(&Params);
  if (NULL == ParsedInputFileContents) {
    result = TYM_NO_INPUT;
  }

  if (TYM_AOK == result) {
    struct TymProgram * ParsedQuery = tym_parse_query(&Params);

    if (TYM_TEST_PARSING == Params.function) {
      print_parsed_program(&Params, ParsedInputFileContents, ParsedQuery);
    } else {
      result = process_program(&Params, ParsedInputFileContents, ParsedQuery);
    }
  }

  tym_fin_str();

  if (TymDefaultSolverTimeout != Params.solver_timeout) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    free((void *)Params.solver_timeout);
#pragma GCC diagnostic pop
  }

  return result;
}
