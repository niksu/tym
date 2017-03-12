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

#define BUF_SIZE 300

struct program_t * parse(const char * string);
char * read_file(char * filename);

char * source_file = NULL;
char * source_file_contents = NULL;
char verbosity = 0;
char * query = NULL;
bool test_parsing = false;

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
      source_file = malloc(strlen(optarg) + 1);
      strcpy(source_file, optarg);
      break;
    case LONG_OPT_VERBOSE:
    case 'v':
      verbosity = 1;
      break;
    case LONG_OPT_QUERY:
    case 'q':
      query = malloc(strlen(optarg) + 1);
      strcpy(query, optarg);
      break;
    case LONG_OPT_TESTPARSING:
      test_parsing = true;
      break;
    // FIXME add support for -h
    default:
      printf("Terminating on unrecognized option\n"); // The offending option would have been reported by getopt by this point.
      return -1;
    }
  }

  if (verbosity) {
    printf("input = %s\n", source_file);
    printf("verbosity = %d\n", verbosity);
    printf("test_parsing = %d\n", test_parsing);
    printf("query = %s\n", query);
  }

  if (NULL != source_file) {
    source_file_contents = read_file(source_file);
    if (test_parsing) {
      printf("input contents |%s|\n", source_file_contents);
    }
    parsed_source_file_contents = parse(source_file_contents);
    if (verbosity && NULL != source_file_contents) {
      printf("input : %d clauses\n", parsed_source_file_contents->no_clauses);
    }
  } else if (test_parsing) {
    printf("(no input file given)\n");
  }

  if (NULL != query) {
    if (test_parsing && !verbosity) {
      printf("query contents |%s|\n", query);
    }
    parsed_query = parse(query);
    if (verbosity && NULL != query) {
      printf("query : %d clauses\n", parsed_query->no_clauses);
    }
  } else if (test_parsing) {
    printf("(no query given)\n");
  }

  if (test_parsing) {
    size_t remaining_buf_size = BUF_SIZE;
    char * buf = (char *)malloc(remaining_buf_size);
    int used_buf_size;

    if (NULL != source_file) {
      used_buf_size = program_to_str(parsed_source_file_contents,
          &remaining_buf_size, buf);
      printf("stringed file contents (size=%d, remaining=%zu)\n|%s|\n",
          used_buf_size, remaining_buf_size, buf);
    }

    if (NULL != query) {
      remaining_buf_size = BUF_SIZE;
      used_buf_size = program_to_str(parsed_query, &remaining_buf_size, buf);
      printf("stringed query (size=%d, remaining=%zu)\n|%s|\n",
          used_buf_size, remaining_buf_size, buf);
    }

    return 0;
  }

  // FIXME add main application logic.

  return 0;
}

char *
read_file(char * filename)
{
  char * contents = NULL;

  // FIXME check whether file exists.
  FILE *file = fopen(filename, "r");
  if (!file) {
    // FIXME complain
  }

  int file_size = -1;
  fseek(file, 0L, SEEK_END);
  file_size = ftell(file);
  rewind(file);

  assert(file_size > 0);

  contents = malloc(file_size + 1);
  fread(contents, sizeof(char), file_size, file);
  contents[file_size] = '\0';

  fclose(file);
  return contents;
}
