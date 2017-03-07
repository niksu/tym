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

void tests(struct clause_t * parsed);
struct program_t * parse(const char * string);

char* source_file = NULL;
char verbosity = 0;
char* query = NULL;

int main (int argc, char **argv) {
  static struct option long_options[] = {
#define LONG_OPT_INPUT 1
    {"input", required_argument, NULL, LONG_OPT_INPUT}, /*  FIXME have "input" and "source_file" be identical? (as parameter and variable names) */
#define LONG_OPT_VERBOSE 2
    {"verbose", no_argument, NULL, LONG_OPT_VERBOSE},
#define LONG_OPT_QUERY 3
    {"query", required_argument, NULL, LONG_OPT_QUERY}
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

  if (source_file) {
    FILE *source_f = fopen(source_file, "r");
    if (!source_f) {
      // FIXME complain
    }

    int file_size = -1;
    fseek(source_f, 0L, SEEK_END);
    file_size = ftell(source_f);
    rewind(source_f);

    printf("file_size=%d", file_size);
    assert(file_size > 0);

    query = malloc(file_size + 1);
    fread(query, sizeof(char), file_size, source_f);

    printf("|%s|", query);

    // FIXME DRY principle
    struct program_t * parsed = parse(query);
    printf("%d clauses\n", parsed->no_clauses);
    size_t SIZE = 300;
    char * buf = (char *)malloc(SIZE);

    int result = program_to_str(parsed, &SIZE, buf);
    printf("stringed query (size=%d, remaining=%zu)\n%s\n", result, SIZE, buf);

    fclose(source_f);
  }

  return 0;
}
