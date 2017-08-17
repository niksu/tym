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
#ifdef TESTING
  tym_test_clause();
  tym_test_formula();
  tym_test_statement();
  exit(0);
#endif

  struct Params Params = {
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
    {"max_var_width", required_argument, NULL, LONG_OPT_MAX_VAR_WIDTH}
  };

  int option_index = 0;
  long v;

  int option;
  while ((option = getopt_long(argc, argv, "hi:q:v", long_options,
          &option_index)) != -1) {
    switch (option) {
    case LONG_OPT_INPUT:
    case 'i':
      Params.input_file = malloc(strlen(optarg) + 1);
      strcpy(Params.input_file, optarg);
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

  if (Params.verbosity > 0) {
    TYM_VERBOSE("input_fine = %s\n", Params.input_file);
    TYM_VERBOSE("verbosity = %d\n", Params.verbosity);
    TYM_VERBOSE("test_parsing = %d\n", Params.test_parsing);
    TYM_VERBOSE("query = %s\n", Params.query);
  }

  struct TymProgram * ParsedInputFileContents = tym_parse_input_file(Params);
  if (NULL == ParsedInputFileContents) {
     return TYM_NO_INPUT;
  }

  struct TymProgram * ParsedQuery = tym_parse_query(Params);

  enum TymReturnCodes result = TYM_AOK;
  if (Params.test_parsing) {
    print_parsed_program(Params, ParsedInputFileContents, ParsedQuery);
  } else {
    result = process_program(Params, ParsedInputFileContents, ParsedQuery);
  }

  return result;
}
