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

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "tym.h"

struct program_t * parse(const char * string);
char * read_file(char * filename);

char * source_file_contents = NULL;

struct param_t params = {
  .source_file = NULL,
  .verbosity = 0,
  .query = NULL,
  .test_parsing = false
};

struct program_t * parsed_source_file_contents = NULL;
struct program_t * parsed_query = NULL;

int
main (int argc, char ** argv)
{
  static struct option long_options[] = {
#define LONG_OPT_INPUT 1
    {"input", required_argument, NULL, LONG_OPT_INPUT}, /*  FIXME have "input" and "source_file" be identical? (as parameter and variable names) */
#define LONG_OPT_VERBOSE 2
    {"verbose", no_argument, NULL, LONG_OPT_VERBOSE},
#define LONG_OPT_QUERY 3
    {"query", required_argument, NULL, LONG_OPT_QUERY},
#define LONG_OPT_TESTPARSING 4
    {"test_parsing", no_argument, NULL, LONG_OPT_TESTPARSING}
  };

  int option_index = 0;

  int option;
  while ((option = getopt_long(argc, argv, "i:vq:", long_options,
          &option_index)) != -1) {
    switch (option) {
    case LONG_OPT_INPUT:
    case 'i':
      params.source_file = malloc(strlen(optarg) + 1);
      strcpy(params.source_file, optarg);
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
    // FIXME add support for -h
    default:
      ERR("Terminating on unrecognized option\n"); // The offending option would have been reported by getopt by this point.
      return -1;
    }
  }

  if (params.verbosity > 0) {
    VERBOSE("input = %s\n", params.source_file);
    VERBOSE("verbosity = %d\n", params.verbosity);
    VERBOSE("test_parsing = %d\n", params.test_parsing);
    VERBOSE("query = %s\n", params.query);
  }

  if (NULL != params.source_file) {
    source_file_contents = read_file(params.source_file);
    if (params.test_parsing) {
      printf("input contents |%s|\n", source_file_contents);
    }
    parsed_source_file_contents = parse(source_file_contents);
    if (params.verbosity > 0 && NULL != source_file_contents) {
      VERBOSE("input : %d clauses\n", parsed_source_file_contents->no_clauses);
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
      VERBOSE("query : %d clauses\n", parsed_query->no_clauses);
    }
  } else if (params.test_parsing) {
    printf("(no query given)\n");
  }

  if (params.test_parsing) {
    size_t remaining_buf_size = BUF_SIZE;
    char * buf = (char *)malloc(remaining_buf_size);
    int used_buf_size;

    if (NULL != params.source_file) {
      used_buf_size = program_to_str(parsed_source_file_contents,
          &remaining_buf_size, buf);
      printf("stringed file contents (size=%d, remaining=%zu)\n|%s|\n",
          used_buf_size, remaining_buf_size, buf);

      free_program(parsed_source_file_contents);
      free(source_file_contents);
      free(params.source_file);
    }

    if (NULL != params.query) {
      remaining_buf_size = BUF_SIZE;
      used_buf_size = program_to_str(parsed_query, &remaining_buf_size, buf);
      printf("stringed query (size=%d, remaining=%zu)\n|%s|\n",
          used_buf_size, remaining_buf_size, buf);

      free_program(parsed_query);
      free(params.query);
    }

    free(buf);

    return 0;
  }

  if (NULL == params.source_file) {
    ERR("No input file given.\n");
  } else if (0 == parsed_source_file_contents->no_clauses) {
    ERR("Input file (%s) is devoid of clauses.\n", params.source_file);
  }


  // FIXME this function is getting too long.


  // FIXME add main application logic.


  DBG("Cleaning up before exiting");

  if (NULL != params.source_file) {
    free_program(parsed_source_file_contents);
    free(source_file_contents);
    free(params.source_file);
  }

  if (NULL != params.query) {
    free_program(parsed_query);
    free(params.query);
  }

  return 0;
}

char *
read_file(char * filename)
{
  assert(NULL != filename);
  DBG("Reading \"%s\"\n", filename);

  char * contents = NULL;

  // FIXME check whether file exists.
  FILE *file = fopen(filename, "r");
  if (NULL == file) {
    // FIXME complain
  }

  int file_size = -1;
  fseek(file, 0L, SEEK_END); // FIXME check return value;
  file_size = ftell(file);
  assert(file_size > 0);
  rewind(file); // FIXME check return value;

  contents = malloc(file_size + 1);
  fread(contents, sizeof(char), file_size, file); // FIXME check return value;
  contents[file_size] = '\0';

  fclose(file); // FIXME check return value;
  return contents;
}
