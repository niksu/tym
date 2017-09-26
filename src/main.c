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
         "   -f, --function FUNCTION \n"
         " Optional parameters: \n"
         "   -q, --query QUERY \n"
         "   -v, --verbose \n"
         "   --test_parsing \n"
         "   --max_var_width N \n"
         "   -h \n", argv_0);
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
    .test_parsing = false
  };

  static struct option long_options[] = {
#define LONG_OPT_INPUT 1
    {"input_file", required_argument, NULL, LONG_OPT_INPUT},
#define LONG_OPT_VERBOSE 2
    {"verbose", no_argument, NULL, LONG_OPT_VERBOSE},
#define LONG_OPT_QUERY 3
    {"query", required_argument, NULL, LONG_OPT_QUERY},
#define LONG_OPT_TESTPARSING 4
    {"test_parsing", no_argument, NULL, LONG_OPT_TESTPARSING},
#define LONG_OPT_MAX_VAR_WIDTH 5
    {"max_var_width", required_argument, NULL, LONG_OPT_MAX_VAR_WIDTH},
#define LONG_OPT_FUNCTION 6
    {"function", required_argument, NULL, LONG_OPT_FUNCTION}
  };

  int option_index = 0;
  long v;

  int option;
  while ((option = getopt_long(argc, argv, "f:hi:q:v", long_options,
          &option_index)) != -1) {
    switch (option) {
    case LONG_OPT_INPUT:
    case 'i':
      Params.input_file = malloc(strlen(optarg) + 1);
      strcpy(Params.input_file, optarg);
      break;
    case LONG_OPT_FUNCTION:
    case 'f':
      Params.function = (enum TymFunction)strtol(optarg, NULL, 10);
      assert(TYM_CONVERT_TO_SMT == Params.function);
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
    case LONG_OPT_TESTPARSING:
      Params.test_parsing = true;
      break;
    case LONG_OPT_MAX_VAR_WIDTH:
      v = strtol(optarg, NULL, 10);
      assert(v <= UINT8_MAX);
      TymMaxVarWidth = (uint8_t)v;
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

  // FIXME check if there remain any unprocess command-line options.

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

    TYM_VERBOSE("input_fine = %s\n", Params.input_file);
    TYM_VERBOSE("verbosity = %d\n", Params.verbosity);
    TYM_VERBOSE("test_parsing = %d\n", Params.test_parsing);
    TYM_VERBOSE("query = %s\n", Params.query);
    TYM_VERBOSE("function = %d\n", Params.function);
  }

  tym_init_str();

  enum TymReturnCodes result = TYM_AOK;

  struct TymProgram * ParsedInputFileContents = tym_parse_input_file(Params);
  if (NULL == ParsedInputFileContents) {
    result = TYM_NO_INPUT;
  }

  if (TYM_AOK == result) {
    struct TymProgram * ParsedQuery = tym_parse_query(Params);

    if (Params.test_parsing) {
      print_parsed_program(Params, ParsedInputFileContents, ParsedQuery);
    } else {
      result = process_program(Params, ParsedInputFileContents, ParsedQuery);
    }
  }

  tym_fin_str();

  return result;
}
