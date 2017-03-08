/*
 * TYM Datalog.
 * Nik Sultana, March 2017.
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include <assert.h>
#include <stdbool.h>

void tests(struct clause_t * parsed);
struct program_t * parse(const char * string);
char * read_file(char * filename);

char* source_file = NULL;
char verbosity = 0;
char* query = NULL;
bool test_parsing = false;

int main (int argc, char **argv) {
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
  while ((option = getopt_long(argc, argv, "i:vq:", long_options, &option_index)) != -1) {
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
    printf("input = %s\n\
verbosity = %d\n\
query = %s\n", source_file, verbosity, query);
    if (query) {
      struct program_t * parsed = parse(query);
      printf("%d clauses\n", parsed->no_clauses);

      size_t SIZE = 300;
      char * buf = (char *)malloc(SIZE);
      int result = program_to_str(parsed, &SIZE, buf);
      printf("stringed query (size=%d, remaining=%zu)\n%s\n", result, SIZE, buf);
    }
  }

  if (test_parsing) {
    query = read_file(source_file);

    printf("|%s|", query);

    // FIXME DRY principle
    struct program_t * parsed = parse(query);
    printf("%d clauses\n", parsed->no_clauses);
    size_t SIZE = 300;
    char * buf = (char *)malloc(SIZE);

    int result = program_to_str(parsed, &SIZE, buf);
    printf("stringed query (size=%d, remaining=%zu)\n%s\n", result, SIZE, buf);
    return 0;
  }

  if (verbosity && source_file) {
    query = read_file(source_file);

    printf("|%s|", query);

    // FIXME DRY principle
    struct program_t * parsed = parse(query);
    printf("%d clauses\n", parsed->no_clauses);
    size_t SIZE = 300;
    char * buf = (char *)malloc(SIZE);

    int result = program_to_str(parsed, &SIZE, buf);
    printf("stringed query (size=%d, remaining=%zu)\n%s\n", result, SIZE, buf);
  }

  return 0;
}

char * read_file(char * filename) {
  char * contents = NULL;
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

  fclose(file);
  return contents;
}
