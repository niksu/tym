/*
 * Support functions for TYM Datalog.
 * Nik Sultana, August 2017.
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifdef TYM_INTERFACE_Z3
#include "interface_z3.h"
#endif
#include "support.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
TYM_DEFINE_LIST_SHALLOW_FREE(stmts, const, struct TymStmts)
#pragma GCC diagnostic pop

static struct TymFmla * solver_invoke(struct TymMdlValuations * vals, struct TymValuation * varmap);
static void solver_loop(struct TymModel ** mdl, struct TymValuation * varmap, struct TymBufferInfo * outbuf);

const char * TymFunctionCommandMapping[] =
  {"test_parsing",
   "smt_output",
   "smt_solve",
   NULL
  };

const char *
tym_functions(void) {
  size_t string_length = 0;
  for (unsigned i = 0; i < TYM_NO_FUNCTION; ++i) {
     string_length += strlen(TymFunctionCommandMapping[i]) + 2;
  }
  char * result = malloc(string_length + 1);
  const char * sep = ", ";
  for (size_t offset = 0, i = 0; i < TYM_NO_FUNCTION; ++i) {
     strcpy(result + offset, TymFunctionCommandMapping[i]);
     offset += strlen(TymFunctionCommandMapping[i]);
     if (TYM_NO_FUNCTION - 1 != i) {
       strcpy(result + offset, sep);
       offset += strlen(sep);
     }
  }

  return result;
}

struct TymProgram *
tym_parse_input_file(struct TymParams Params)
{
  struct TymProgram * result = NULL;
  if (NULL != Params.input_file) {
    char * InputFileContents = read_file(Params.input_file);
    if (TYM_TEST_PARSING == Params.function) {
      printf("input contents |%s|\n", InputFileContents);
    }
    result = parse(InputFileContents);
    if (Params.verbosity > 0 && NULL != InputFileContents) {
      TYM_VERBOSE("input : %d clauses\n", result->no_clauses);
    }
    free(InputFileContents);
  } else if (TYM_TEST_PARSING == Params.function) {
    printf("(no input file given)\n");
  }
  return result;
}

struct TymProgram *
tym_parse_query(struct TymParams Params)
{
  struct TymProgram * result = NULL;
  if (NULL != Params.query) {
    if ((TYM_TEST_PARSING == Params.function) && 0 == Params.verbosity) {
      printf("query contents |%s|\n", Params.query);
    }
    result = parse(Params.query);
    if (Params.verbosity > 0 && NULL != Params.query) {
      TYM_VERBOSE("query : %d clauses\n", result->no_clauses);
    }
  } else if (TYM_TEST_PARSING == Params.function) {
    printf("(no query given)\n");
  }
  return result;
}

void
print_parsed_program(struct TymParams Params, struct TymProgram * ParsedInputFileContents,
  struct TymProgram * ParsedQuery)
{
  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  if (NULL != Params.input_file) {
    res = tym_program_str(ParsedInputFileContents, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    TYM_DBG_PRINT_BUFFER(printf, outbuf, "stringed file contents")
    tym_free_program(ParsedInputFileContents);
    free(Params.input_file);
  }

  if (NULL != Params.query) {
    tym_reset_buffer(outbuf);
    res = tym_program_str(ParsedQuery, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    TYM_DBG_PRINT_BUFFER(printf, outbuf, "stringed query")
    tym_free_program(ParsedQuery);
    free(Params.query);
  }

  tym_free_buffer(outbuf);
}

static struct TymFmla *
solver_invoke(struct TymMdlValuations * vals, struct TymValuation * varmap)
{
  struct TymFmla * found_model = NULL;
  tym_z3_check();
  enum TymSatisfiable result = tym_z3_satisfied();
#if TYM_DEBUG
  printf("sat=%d\n", (int)result);
#endif
  switch (result) {
  case TYM_SAT_YES:
#if TYM_DEBUG
    tym_z3_print_model();
#endif
    tym_z3_get_model(vals);
    // Map the constant back to the variable in the query.
    for (unsigned i = 0; i < vals->count; i++) {
      const struct TymValuation * varmap_cursor = varmap;
      while (NULL != varmap_cursor) {
        if (0 == tym_cmp_str(vals->v[i].name, varmap_cursor->var)) {
          struct TymTerm * val = varmap_cursor->val;
          assert(TYM_VAR == val->kind);
//          vals->v[i].name = TYM_STR_DUPLICATE(val->identifier);
          break;
        }
        varmap_cursor = varmap_cursor->next;
      }

      assert(NULL != varmap_cursor);

      // FIXME conjoin the eq's
      struct TymFmla * atom =
        tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE(tym_eqK), 2,
//            tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(vals->v[i].name)),  
            tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(varmap_cursor->var)),
            tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(vals->v[i].value)));

      if (NULL == found_model) {
        found_model = atom;
      } else {
        found_model = tym_mk_fmla_and(found_model, atom);
      }
    }
    tym_z3_print_valuations(vals);

    for (unsigned i = 0; i < vals->count; i++) {
      tym_free_str(vals->v[i].value);
      vals->v[i].value = NULL;
    }
    break;
  case TYM_SAT_NO:
  case TYM_SAT_UNKNOWN:
    break;
  default:
    assert(0);
  }
  return found_model;
}

static void
solver_loop(struct TymModel ** mdl, struct TymValuation * varmap, struct TymBufferInfo * outbuf)
{
#ifndef TYM_INTERFACE_Z3
  assert(0); // Cannot run solver in this build mode.
#else
  tym_z3_begin();
  tym_z3_assert_smtlib2(tym_buffer_contents(outbuf));

  size_t num_vars = tym_valuation_len(varmap);
  const TymStr ** cs = malloc(sizeof(*cs) * (num_vars + 1));
  const struct TymValuation * varmap_cursor = varmap;
  for (unsigned i = 0; i < (unsigned)num_vars; i++) {
    cs[i] = varmap_cursor->var;
    varmap_cursor = varmap_cursor->next;
  }
  cs[num_vars] = NULL;
  struct TymMdlValuations * vals = tym_z3_mk_valuations(cs);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;
  struct TymFmla * found_model = NULL;
  while (1) {
    found_model = solver_invoke(vals, varmap);
    if (NULL == found_model) {
      break;
    } else {
      struct TymStmt * stmt = tym_mk_stmt_axiom(tym_mk_fmla_not(found_model));

//      tym_reset_buffer(outbuf);
//      res = tym_stmt_str(stmt, outbuf);
//      assert(tym_is_ok_TymBufferWriteResult(res));
//      free(res);
//printf("new statement: %s\n", tym_buffer_contents(outbuf));    

      tym_strengthen_model(*mdl, stmt);

      // FIXME annoying
    struct TymStmts * reordered_stmts =
      tym_order_statements((*mdl)->stmts);
    tym_shallow_free_stmts((*mdl)->stmts);
    (*mdl)->stmts = reordered_stmts;

// FIXME DRY?  
      tym_reset_buffer(outbuf);
//      res = tym_stmt_str(stmt, outbuf);
      res = tym_model_str(*mdl, outbuf); // FIXME ideally make Z3 work incrementally instead of giving it the whole model each time.
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
//printf("here it is: %s\n", tym_buffer_contents(outbuf));    
      tym_z3_assert_smtlib2(tym_buffer_contents(outbuf));

//      tym_free_stmt(stmt); -- this will be freed when the model is freed.
    }
  }

  tym_z3_free_valuations(vals);  
  free(cs);  
  tym_z3_end();
#endif // TYM_INTERFACE_Z3
}

enum TymReturnCodes
process_program(struct TymParams Params, struct TymProgram * ParsedInputFileContents,
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
  *vg = tym_mk_sym_gen(TYM_CSTR_DUPLICATE("V"));

  struct TymSymGen * cg = tym_mk_sym_gen(TYM_CSTR_DUPLICATE("c"));

  struct TymModel * mdl = NULL;
  if (NULL != ParsedInputFileContents) {
    mdl = tym_translate_program(ParsedInputFileContents, vg);
    tym_statementise_universe(mdl);
  }

  struct TymValuation * varmap = NULL;
  if (NULL != ParsedQuery &&
      // If mdl is NULL then it means that the universe is empty, and there's nothing to be reasoned about.
      NULL != mdl) {
    varmap = tym_translate_query(ParsedQuery, mdl, cg);
  }
#if TYM_DEBUG
  else {
    printf("(No query is being printed, since none was given as a parameter)\n");
  }
#endif

  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  if (NULL != mdl) {
#if TYM_DEBUG
    tym_reset_buffer(outbuf);
    res = tym_model_str(mdl, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    TYM_DBG_BUFFER(outbuf, "PREmodel")
#endif

    struct TymStmts * reordered_stmts =
      tym_order_statements(mdl->stmts);
    tym_shallow_free_stmts(mdl->stmts);
    mdl->stmts = reordered_stmts;

    tym_reset_buffer(outbuf);
    res = tym_model_str(mdl, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    TYM_DBG_BUFFER(outbuf, "model")

    if (TYM_CONVERT_TO_SMT == Params.function) {
      printf("%s", tym_buffer_contents(outbuf));
    } else if (TYM_CONVERT_TO_SMT_AND_SOLVE == Params.function) {
      solver_loop(&mdl, varmap, outbuf);
    }
  }

  if (NULL != varmap) {
    tym_free_valuation(varmap);
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
