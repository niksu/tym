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

struct Params {
  char * input_file;
  char verbosity;
  char * query;
  bool test_parsing;
};

enum TymReturnCodes {TYM_AOK=0, TYM_UNRECOGNISED_PARAMETER, TYM_NO_INPUT, TYM_INVALID_INPUT};

struct TymProgram * parse(const char * string);
static char * read_file(char * filename);
struct TymProgram * tym_parse_input_file(struct Params Params);
struct TymProgram * tym_parse_query(struct Params Params);
static void print_parsed_program(struct Params Params, struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);
static enum TymReturnCodes process_program(struct Params Params, struct TymProgram * ParsedInputFileContents, struct TymProgram * ParsedQuery);

TYM_DECLARE_LIST_SHALLOW_FREE(stmts, const, struct TymStmts)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
TYM_DEFINE_LIST_SHALLOW_FREE(stmts, const, struct TymStmts)
#pragma GCC diagnostic pop

struct TymProgram *
tym_parse_input_file(struct Params Params)
{
  struct TymProgram * result = NULL;
  if (NULL != Params.input_file) {
    char * InputFileContents = read_file(Params.input_file);
    if (Params.test_parsing) {
      printf("input contents |%s|\n", InputFileContents);
    }
    result = parse(InputFileContents);
    if (Params.verbosity > 0 && NULL != InputFileContents) {
      TYM_VERBOSE("input : %d clauses\n", result->no_clauses);
    }
    free(InputFileContents);
  } else if (Params.test_parsing) {
    printf("(no input file given)\n");
  }
  return result;
}

// FIXME DRY principle wrt tym_parse_input_file()
struct TymProgram *
tym_parse_query(struct Params Params)
{
  struct TymProgram * result = NULL;
  if (NULL != Params.query) {
    if (Params.test_parsing && 0 == Params.verbosity) {
      printf("query contents |%s|\n", Params.query);
    }
    result = parse(Params.query);
    if (Params.verbosity > 0 && NULL != Params.query) {
      TYM_VERBOSE("query : %d clauses\n", result->no_clauses);
    }
  } else if (Params.test_parsing) {
    printf("(no query given)\n");
  }
  return result;
}

static void
print_parsed_program(struct Params Params, struct TymProgram * ParsedInputFileContents,
  struct TymProgram * ParsedQuery)
{
  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  if (NULL != Params.input_file) {
    res = tym_program_to_str(ParsedInputFileContents, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("stringed file contents (size=%lu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);

    tym_free_program(ParsedInputFileContents);
    free(Params.input_file);
  }

  if (NULL != Params.query) {
    res = tym_program_to_str(ParsedQuery, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("stringed query (size=%lu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);

    tym_free_program(ParsedQuery);
    free(Params.query);
  }

  tym_free_buffer(outbuf);
}

static enum TymReturnCodes
process_program(struct Params Params, struct TymProgram * ParsedInputFileContents,
  struct TymProgram * ParsedQuery)
{
  if (NULL == Params.input_file) {
    TYM_ERR("No input file given.\n");
    return TYM_INVALID_INPUT;
  } else if (0 == ParsedInputFileContents->no_clauses) {
    TYM_ERR("Input file (%s) is devoid of clauses.\n", Params.input_file);
    return TYM_INVALID_INPUT;
  }

  struct TymSymGen ** vg = malloc(sizeof *vg);
  *vg = NULL;
  *vg = tym_mk_sym_gen(strdup("V"));

  struct TymSymGen * cg = tym_mk_sym_gen(strdup("c"));

  struct TymModel * mdl = NULL;
  if (NULL != ParsedInputFileContents) {
    mdl = tym_translate_program(ParsedInputFileContents, vg);
    tym_statementise_universe(mdl);
  }

  if (NULL != ParsedQuery &&
      // If mdl is NULL then it means that the universe is empty, and there's nothing to be reasoned about.
      NULL != mdl) {
    tym_translate_query(ParsedQuery, mdl, cg);
  }
#if DEBUG
  else {
    printf("(No query is being printed, since none was given as a parameter)\n");
  }
#endif

  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  if (NULL != mdl) {
#if DEBUG
    res = tym_model_str(mdl, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("PREmodel (size=%zu, remaining=%zu)\n|%s|\n",
        outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
#endif

    const struct TymStmts * reordered_stmts = tym_order_statements(mdl->stmts);
    tym_shallow_free_stmts(mdl->stmts);
    mdl->stmts = reordered_stmts;

    res = tym_model_str(mdl, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("model (size=%zu, remaining=%zu)\n|%s|\n",
        outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  }

  TYM_DBG("Cleaning up before exiting\n");

  if (NULL != mdl) {
    tym_free_model(mdl);
  }
  tym_free_sym_gen(*vg);
  free(vg);
  tym_free_sym_gen(cg);

  tym_free_buffer(outbuf);

  if (NULL != Params.input_file) {
    tym_free_program(ParsedInputFileContents);
    free(Params.input_file);
  }

  if (NULL != Params.query) {
    tym_free_program(ParsedQuery);
    free(Params.query);
  }

  return TYM_AOK;
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
  while ((option = getopt_long(argc, argv, "i:vq:", long_options,
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
    // FIXME add support for -h
    default:
      TYM_ERR("Terminating on unrecognized option\n"); // The offending option would have been reported by getopt by this point.
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

static char *
read_file(char * filename)
{
  assert(NULL != filename);
  TYM_DBG("Reading \"%s\"\n", filename);

  char * contents = NULL;

  FILE * file = fopen(filename, "r");
  assert (NULL != file);

  assert(0 == fseek(file, 0L, SEEK_END));
  long pre_file_size = ftell(file);
  assert (pre_file_size >= 0);

  size_t file_size = (size_t)pre_file_size;
  assert(file_size > 0);
  rewind(file);

  contents = malloc(file_size + 1);
  size_t post_file_size = fread(contents, sizeof(char), file_size, file);
  assert(post_file_size == file_size);
  contents[file_size] = '\0';

  assert(0 == fclose(file));
  return contents;
}
