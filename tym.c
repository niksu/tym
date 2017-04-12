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
#include "formula.h"
#include "parser.h"
#include "lexer.h"
#include "statement.h"
#include "symbols.h"
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

void test_clause(void);
void test_formula(void);
void test_statement(void);

struct fmla_t *
translate_atom (struct atom_t * at)
{
  assert(NULL != at);
  char ** args = malloc(sizeof(char *) * at->arity);
  for (int i = 0; i < at->arity; i++) {
    args[i] = (char *)at->args[i].identifier;
  }
  return mk_fmla_atom(at->predicate, at->arity, args);
}

struct fmla_t *
translate_body (struct clause_t * cl)
{
  struct fmlas_t * fmlas = NULL;
  for (int i = 0; i < cl->body_size; i++) {
    fmlas = mk_fmla_cell(translate_atom(cl->body + i), fmlas);
  }
  return mk_fmla_ands(fmlas);
}

struct fmla_t *
translate_bodies (struct clauses_t * cls)
{
  struct clauses_t * cursor = cls;
  struct fmlas_t * fmlas = NULL;
  while (NULL != cursor) {
    fmlas = mk_fmla_cell(translate_body(cursor->clause), fmlas);
    cursor = cursor->next;
  }
  return mk_fmla_ors(fmlas);
}

int
main (int argc, char ** argv)
{
#ifdef TESTING
  test_clause();
  test_formula();
  test_statement();
  exit(0);
#endif
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
    char * buf = malloc(remaining_buf_size);
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


  struct atom_database_t * adb = mk_atom_database();

  for (int i = 0; i < parsed_source_file_contents->no_clauses; i++) {
    (void)clause_database_add(parsed_source_file_contents->program[i], adb, NULL);
  }
  size_t remaining_buf_size = BUF_SIZE;
  char * buf = malloc(remaining_buf_size);
  atom_database_str(adb, &remaining_buf_size, buf);
  printf("clause database (remaining=%zu)\n|%s|\n", remaining_buf_size, buf);


  // 1. Generate prologue: universe sort, and its inhabitants.
  struct model_t * m = mk_model(mk_universe(adb->tdb->herbrand_universe));

  remaining_buf_size = BUF_SIZE;
  size_t l = model_str(m, &remaining_buf_size, buf);
  printf("model (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  // NOTE if we don't do this, remaining_buf_size will become 0 causing some
  //      output to be dropped, then it might wrap back and output will resume,
  //      so best to keep it topped up.
  // FIXME maybe should simply remove this parameter then, if it needs maintenance and doesn't yield obvious gain?
  remaining_buf_size = BUF_SIZE;

  // 2. Add axiom characterising the provability of all elements of the Hilbert base.
  char * vK = malloc(sizeof(char) * 2);
  strcpy(vK, "V");
  struct var_gen_t * vg = mk_var_gen(vK);
  struct predicates_t * preds_cursor = atom_database_to_predicates(adb);
  while (NULL != preds_cursor) {
    //predicate_str(preds_cursor->predicate, &remaining_buf_size, buf);
    //printf("%s ", buf);
//array of formulas
//  universally quantified
//  head (with variables as terms) "if" disjunction of bodies (conjunctions) (with conjoined variable evaluations)

    printf("no_bodies = %zu\n", num_predicate_bodies(preds_cursor->predicate));

    size_t out_size;

    struct fmla_t * fmla = translate_bodies(preds_cursor->predicate->bodies);
    remaining_buf_size = BUF_SIZE;
    out_size = fmla_str(fmla, &remaining_buf_size, buf);
    assert(out_size > 0);
    printf("translated: %s\n", buf);

    if (NULL == preds_cursor->predicate->bodies) {
      // "No bodies" means that the atom never appears as the head of a clause.

      char ** var_args = malloc(sizeof(char **) * preds_cursor->predicate->arity);

      for (int i = 0; i < preds_cursor->predicate->arity; i++) {
        var_args[i] = mk_new_var(vg);
      }

      struct fmla_t * atom = mk_fmla_atom((char *)preds_cursor->predicate->predicate,
          preds_cursor->predicate->arity, var_args);

      out_size = fmla_str(atom, &remaining_buf_size, buf);
      assert(out_size > 0);
      printf("bodyless: %s\n", buf);

      free_fmla(atom);
    } else {
      struct clauses_t * body_cursor = preds_cursor->predicate->bodies;
      while (NULL != body_cursor) {
        printf(">");

        struct var_gen_t * vg_copy = copy_var_gen(vg);

        struct atom_t * head_atom = &(body_cursor->clause->head);
        char ** args = malloc(sizeof(char **) * head_atom->arity);

        for (int i = 0; i < head_atom->arity; i++) {
          size_t buf_size = 20/* FIXME const */;
          args[i] = malloc(sizeof(char) * buf_size);
          int l_sub = term_to_str(&(head_atom->args[i]), &buf_size, args[i]);
          if (l_sub < 0) {
            // FIXME complain
          }
        }

        // Abstract the atom's parameters.
        struct fmla_t * head_fmla = mk_fmla_atom(head_atom->predicate, head_atom->arity, args);

        out_size = fmla_str(head_fmla, &remaining_buf_size, buf);
        assert(out_size > 0);
        printf("from: %s\n", buf);

        struct valuation_t ** v = malloc(sizeof(struct valuation_t **));
        struct fmla_t * abs_head_fmla = mk_abstract_vars(head_fmla, vg_copy, v);
        out_size = fmla_str(abs_head_fmla, &remaining_buf_size, buf);
        assert(out_size > 0);
        printf("to: %s\n", buf);

        out_size = valuation_str(*v, &remaining_buf_size, buf);
        if (0 == out_size) {
          printf("  where: (no substitutions)\n");
        } else {
          printf("  where: %s\n", buf);
        }

        free_fmla(head_fmla);
        free_fmla(abs_head_fmla);
        if (NULL != *v) {
          // i.e., the predicate isn't nullary.
          free_valuation(*v);
        }
        free(v);

        body_cursor = body_cursor->next;
        if (NULL == body_cursor) {
          struct var_gen_t * tmp = vg;
          vg = vg_copy;
          vg_copy = tmp;
        }
        free_var_gen(vg_copy);
      }
    }

    preds_cursor = preds_cursor->next;

    printf("\n");
  }


  DBG("Cleaning up before exiting\n");

  free_var_gen(vg);

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
