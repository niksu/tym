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

#include "libtym.h"
#include "module_tests.h"

struct TymProgram * parse(const char * string);
char * read_file(char * filename);

char * input_file_contents = NULL;

struct param_t {
  char * input_file;
  char verbosity;
  char * query;
  bool test_parsing;
};

struct param_t params = {
  .input_file = NULL,
  .verbosity = 0,
  .query = NULL,
  .test_parsing = false
};

struct TymProgram * parsed_input_file_contents = NULL;
struct TymProgram * parsed_query = NULL;

TYM_DECLARE_LIST_SHALLOW_FREE(stmts, const, struct stmts_t)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
TYM_DEFINE_LIST_SHALLOW_FREE(stmts, const, struct stmts_t)
#pragma GCC diagnostic pop

int
main(int argc, char ** argv)
{
#ifdef TESTING
  tym_test_clause();
  tym_test_formula();
  tym_test_statement();
  exit(0);
#endif
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
      params.input_file = malloc(strlen(optarg) + 1);
      strcpy(params.input_file, optarg);
      break;
    case LONG_OPT_VERBOSE:
    case 'v':
      params.verbosity = 1;
      break;
    case LONG_OPT_QUERY:
    case 'q':
      params.query = malloc(strlen(optarg) + 1);
      strcpy(params.query, optarg);
      break;
    case LONG_OPT_TESTPARSING:
      params.test_parsing = true;
      break;
    case LONG_OPT_MAX_VAR_WIDTH:
      v = strtol(optarg, NULL, 10);
      assert(v <= UINT8_MAX);
      TymMaxVarWidth = (uint8_t)v;
      break;
    // FIXME add support for -h
    default:
      TYM_ERR("Terminating on unrecognized option\n"); // The offending option would have been reported by getopt by this point.
      return -1;
    }
  }

  if (params.verbosity > 0) {
    TYM_VERBOSE("input_fine = %s\n", params.input_file);
    TYM_VERBOSE("verbosity = %d\n", params.verbosity);
    TYM_VERBOSE("test_parsing = %d\n", params.test_parsing);
    TYM_VERBOSE("query = %s\n", params.query);
  }

  if (NULL != params.input_file) {
    input_file_contents = read_file(params.input_file);
    if (params.test_parsing) {
      printf("input contents |%s|\n", input_file_contents);
    }
    parsed_input_file_contents = parse(input_file_contents);
    if (params.verbosity > 0 && NULL != input_file_contents) {
      TYM_VERBOSE("input : %d clauses\n", parsed_input_file_contents->no_clauses);
    }
  } else if (params.test_parsing) {
    printf("(no input file given)\n");
  }

  if (NULL != params.query) {
    if (params.test_parsing && 0 == params.verbosity) {
      printf("query contents |%s|\n", params.query);
    }
    parsed_query = parse(params.query);
    if (params.verbosity > 0 && NULL != params.query) {
      TYM_VERBOSE("query : %d clauses\n", parsed_query->no_clauses);
    }
  } else if (params.test_parsing) {
    printf("(no query given)\n");
  }

  if (params.test_parsing) {
    struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
    struct TymBufferWriteResult * res = NULL;

    if (NULL != params.input_file) {
      res = tym_program_to_str(parsed_input_file_contents, outbuf);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
      printf("stringed file contents (size=%lu, remaining=%zu)\n|%s|\n",
        outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);

      tym_free_program(parsed_input_file_contents);
      free(input_file_contents);
      free(params.input_file);
    }

    if (NULL != params.query) {
      res = tym_program_to_str(parsed_query, outbuf);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
      printf("stringed query (size=%lu, remaining=%zu)\n|%s|\n",
        outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);

      tym_free_program(parsed_query);
      free(params.query);
    }

    tym_free_buffer(outbuf);

    return 0;
  }

  if (NULL == params.input_file) {
    TYM_ERR("No input file given.\n");
  } else if (0 == parsed_input_file_contents->no_clauses) {
    TYM_ERR("Input file (%s) is devoid of clauses.\n", params.input_file);
  }

  struct TymSymGen ** vg = malloc(sizeof(struct TymSymGen *));
  *vg = NULL;
  *vg = tym_mk_sym_gen(strdup("V"));

  struct TymSymGen * cg = tym_mk_sym_gen(strdup("c"));

  struct model_t * mdl = NULL;
  if (NULL != parsed_input_file_contents) {
    mdl = translate_program(parsed_input_file_contents, vg);
    statementise_universe(mdl);
  }

  if (NULL != parsed_query &&
      // If mdl is NULL then it means that the universe is empty, and there's nothing to be reasoned about.
      NULL != mdl) {
    translate_query(parsed_query, mdl, cg);
  }
#if DEBUG
  else {
    printf("(No query is being printed, since none was given as a parameter)\n");
  }
#endif

  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TymBufferWriteResult * res = NULL;

  if (NULL != mdl) {
#if DEBUG
    res = model_str(mdl, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("PREmodel (size=%zu, remaining=%zu)\n|%s|\n",
        outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
#endif

    const struct stmts_t * reordered_stmts = order_statements(mdl->stmts);
    tym_shallow_free_stmts(mdl->stmts);
    mdl->stmts = reordered_stmts;

    res = model_str(mdl, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("model (size=%zu, remaining=%zu)\n|%s|\n",
        outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  }

  TYM_DBG("Cleaning up before exiting\n");

  if (NULL != mdl) {
    free_model(mdl);
  }
  tym_free_sym_gen(*vg);
  free(vg);
  tym_free_sym_gen(cg);

  tym_free_buffer(outbuf);

  if (NULL != params.input_file) {
    tym_free_program(parsed_input_file_contents);
    free(input_file_contents);
    free(params.input_file);
  }

  if (NULL != params.query) {
    tym_free_program(parsed_query);
    free(params.query);
  }

  return 0;
}

char *
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
