/*
Copyright Nik Sultana, 2019

This file is part of TYM Datalog. (https://www.github.com/niksu/tym)

TYM Datalog is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TYM Datalog is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details, a copy of which
is included in the file called LICENSE distributed with TYM Datalog.

You should have received a copy of the GNU Lesser General Public License
along with TYM Datalog.  If not, see <https://www.gnu.org/licenses/>.


This file: Support functions for TYM Datalog.
*/

#ifdef TYM_INTERFACE_Z3
#include "interface_z3.h"
#endif
#include "interface_c.h"
#include "output_c.h"
#include "support.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
TYM_DEFINE_LIST_SHALLOW_FREE(stmts, const, struct TymStmts)
#pragma GCC diagnostic pop

#ifdef TYM_INTERFACE_Z3
static struct TymFmla * solver_invoke(struct TymParams *, struct TymProgram *, struct TymMdlValuations *, struct TymValuation *, struct TymBufferInfo *);
static void solver_loop(struct TymParams *, struct TymModel **, struct TymValuation *, struct TymProgram *, struct TymBufferInfo *);
static const struct TymValuation * find_valuation_for(const TymStr *, struct TymValuation *);
#endif
static const char * tym_show_choices(const char ** choices, const unsigned choice_terminator);

enum TymModelOutput TymDefaultModelOutput = TYM_MODEL_OUTPUT_VALUATION;
const char * const TymDefaultSolverTimeout = "10000";

enum TymSatisfiable TymState_LastSolverResult = TYM_SAT_NONE;

const char * TymFunctionCommandMapping[] =
  {"nothing",
   "test_parsing",
   "smt_output",
   "smt_solve",
   "c_output",
   "dump_hilbert_universe",
   "dump_atoms",
   NULL
  };

const char * TymModelOutputCommandMapping[] =
  {"valuation",
   "fact",
   "all",
   NULL
  };

const char *
tym_show_choices(const char ** choices, const unsigned choice_terminator)
{
  size_t string_length = 0;
  for (unsigned i = 0; i < choice_terminator; ++i) {
     string_length += strlen(choices[i]) + 2;
  }
  char * result = malloc(string_length + 1);
  const char * sep = ", ";
  for (size_t offset = 0, i = 0; i < choice_terminator; ++i) {
     strcpy(result + offset, choices[i]);
     offset += strlen(choices[i]);
     if (choice_terminator - 1 != i) {
       strcpy(result + offset, sep);
       offset += strlen(sep);
     }
  }

  return result;
}

const char *
tym_functions(void)
{
  return tym_show_choices(TymFunctionCommandMapping, TYM_NO_FUNCTION);
}

const char *
tym_model_outputs(void)
{
  return tym_show_choices(TymModelOutputCommandMapping, TYM_NO_MODEL_OUTPUT);
}

struct TymProgram *
tym_parse_input_file(struct TymParams * Params)
{
  struct TymProgram * result = NULL;
  if (NULL != Params->input_file) {
    char * InputFileContents = read_file(Params->input_file);
    if (TYM_TEST_PARSING == Params->function) {
      printf("input contents |%s|\n", InputFileContents);
    }
    result = parse(InputFileContents);
    if (Params->verbosity > 0 && NULL != InputFileContents) {
      TYM_VERBOSE("input : %d clauses\n", result->no_clauses);
    }
    free(InputFileContents);
  } else if (TYM_TEST_PARSING == Params->function) {
    printf("(no input file given)\n");
  }
  return result;
}

struct TymProgram *
tym_parse_query(struct TymParams * Params)
{
  struct TymProgram * result = NULL;
  if (NULL != Params->query) {
    if ((TYM_TEST_PARSING == Params->function) && 0 == Params->verbosity) {
      printf("query contents |%s|\n", Params->query);
    }
    result = parse(Params->query);
    if (Params->verbosity > 0 && NULL != Params->query) {
      TYM_VERBOSE("query : %d clauses\n", result->no_clauses);
    }
  } else if (TYM_TEST_PARSING == Params->function) {
    printf("(no query given)\n");
  }
  return result;
}

enum TymReturnCode
print_parsed_program(struct TymParams * Params, struct TymProgram * ParsedInputFileContents,
  struct TymProgram * ParsedQuery)
{
  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  if (NULL != Params->input_file) {
    res = tym_program_str(ParsedInputFileContents, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    TYM_DBG_PRINT_BUFFER(printf, outbuf, "stringed file contents")
  }

  if (NULL != Params->query) {
    tym_reset_buffer(outbuf);
    res = tym_program_str(ParsedQuery, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    TYM_DBG_PRINT_BUFFER(printf, outbuf, "stringed query")
  }

  tym_free_buffer(outbuf);
  return TYM_AOK;
}

#ifdef TYM_INTERFACE_Z3
static const struct TymValuation *
find_valuation_for(const TymStr * var_name, struct TymValuation * varmap)
{
  const struct TymValuation * varmap_cursor = varmap;
  while (NULL != varmap_cursor) {
    if (0 == tym_cmp_str(var_name, varmap_cursor->var)) {
      struct TymTerm * val = varmap_cursor->val;
      assert(TYM_VAR == val->kind);
      // If we could change "vals", this line would map the constant's name
      // back to the variable name that was used in the user's query:
      // vals->v[i].name = TYM_STR_DUPLICATE(val->identifier);
      break;
    }
    varmap_cursor = varmap_cursor->next;
  }
  return varmap_cursor;
}

static struct TymFmla *
solver_invoke(struct TymParams * params, struct TymProgram * ParsedQuery, struct TymMdlValuations * vals, struct TymValuation * varmap, struct TymBufferInfo * result_outbuf)
{
  struct TymFmla * found_model = NULL;
  tym_z3_check();
  TymState_LastSolverResult = tym_z3_satisfied();
#if TYM_DEBUG
  printf("sat=%d\n", (int)result);
#endif
  switch (TymState_LastSolverResult) {
  case TYM_SAT_YES:
#if TYM_DEBUG
    tym_z3_print_model();
#endif
    tym_z3_get_model(vals);
    for (unsigned i = 0; i < vals->count; i++) {
      const struct TymValuation * mapped_var =
        find_valuation_for(vals->v[i].const_name, varmap);
      assert(NULL != mapped_var);
      struct TymFmla * atom =
        tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE(tym_eqK), 2,
            tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(mapped_var->var)),
            tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(vals->v[i].value)));

      if (NULL == found_model) {
        found_model = atom;
      } else {
        found_model = tym_mk_fmla_and(found_model, atom);
      }
    }

    if (TYM_MODEL_OUTPUT_VALUATION == params->model_output ||
        TYM_ALL_MODEL_OUTPUT == params->model_output) {
      tym_mdl_print_valuations(vals);
    }

    if (TYM_MODEL_OUTPUT_FACT == params->model_output ||
        TYM_ALL_MODEL_OUTPUT == params->model_output) {
      struct TymProgram * instance = tym_mdl_instantiate_valuation(ParsedQuery, vals);

      struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;
      tym_reset_buffer(result_outbuf);
      res = tym_program_str(instance, result_outbuf);
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
      printf("%s\n", tym_buffer_contents(result_outbuf));
      tym_free_program(instance);
    }

    tym_mdl_reset_valuations(vals);
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
solver_loop(struct TymParams * params, struct TymModel ** mdl, struct TymValuation * varmap, struct TymProgram * ParsedQuery, struct TymBufferInfo * outbuf)
{
  tym_z3_begin(params);
  tym_z3_assert_smtlib2(tym_buffer_contents(outbuf));

  size_t num_vars = tym_valuation_len(varmap);
  const TymStr ** consts = malloc(sizeof(*consts) * (num_vars + 1));
  const TymStr ** vars = malloc(sizeof(*vars) * (num_vars + 1));
  const struct TymValuation * varmap_cursor = varmap;
  for (unsigned i = 0; i < (unsigned)num_vars; i++) {
    consts[i] = varmap_cursor->var;
    vars[i] = varmap_cursor->val->identifier;
    varmap_cursor = varmap_cursor->next;
  }
  consts[num_vars] = NULL;
  vars[num_vars] = NULL;
  struct TymMdlValuations * vals = tym_mdl_mk_valuations(consts, vars);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;
  struct TymFmla * found_model = NULL;

  struct TymBufferInfo * result_outbuf = tym_mk_buffer(TYM_BUF_SIZE);

  while (1) {
#if TYM_DEBUG
    printf("Invoking...\n");
#endif
    found_model = solver_invoke(params, ParsedQuery, vals, varmap, result_outbuf);
#if TYM_DEBUG
    printf("...Invoked\n");
#endif
    if (NULL == found_model) {
      break;
    } else {
      struct TymStmt * stmt = tym_mk_stmt_axiom(tym_mk_fmla_not(found_model));

      tym_strengthen_model(*mdl, stmt);

      // FIXME annoying -- but necessary in general every time the model is updated?
      struct TymStmts * reordered_stmts =
        tym_order_statements((*mdl)->stmts);
      tym_shallow_free_stmts((*mdl)->stmts);
      (*mdl)->stmts = reordered_stmts;

      tym_reset_buffer(outbuf);
      res = tym_model_str(*mdl, outbuf); // FIXME ideally make Z3 work incrementally instead of giving it the whole model each time.
      assert(tym_is_ok_TymBufferWriteResult(res));
      free(res);
      tym_z3_assert_smtlib2(tym_buffer_contents(outbuf));
    }
  }

  tym_free_buffer(result_outbuf);
  tym_mdl_free_valuations(vals);
  free(consts);
  free(vars);
  tym_z3_end();
}
#endif // TYM_INTERFACE_Z3

enum TymReturnCode
process_program(struct TymParams * Params, struct TymProgram * ParsedInputFileContents,
  struct TymProgram * ParsedQuery)
{
  if (NULL == Params->input_file) {
    TYM_ERR("No input file given.\n");
    return TYM_INVALID_INPUT;
  } else if (0 == ParsedInputFileContents->no_clauses) {
    TYM_ERR("Input file (%s) is devoid of clauses.\n", Params->input_file);
    return TYM_INVALID_INPUT;
  }

  struct TymSymGen ** vg = malloc(sizeof *vg);
  *vg = NULL;
  *vg = tym_mk_sym_gen(TYM_CSTR_DUPLICATE("V"));

  struct TymSymGen * cg = tym_mk_sym_gen(TYM_CSTR_DUPLICATE("c"));

  struct TymModel * mdl = NULL;
  struct TymAtomDatabase * adb = tym_mk_atom_database();
  if (NULL != ParsedInputFileContents) {
    mdl = tym_translate_program(ParsedInputFileContents, vg, adb);
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

  if (TYM_DUMP_HILBERT_UNIVERSE == Params->function) {
    tym_reset_buffer(outbuf);
    res = tym_term_database_dump(adb->tdb, outbuf);
//    res = tym_term_database_str(adb->tdb, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("Hilbert universe:\n");
//    printf("  %s\n", tym_buffer_contents(outbuf));
    tym_reset_read_idx(outbuf);
    do {
      printf("  %s\n", tym_buffer_contents(outbuf));
    } while(tym_progress_next_entry(outbuf));
  } else if (TYM_DUMP_ATOMS == Params->function) {
    tym_reset_buffer(outbuf);
    res = tym_atom_database_str(adb, outbuf);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    printf("Atoms:\n%s", tym_buffer_contents(outbuf));
  } else if (TYM_CONVERT_TO_C == Params->function) {
    emit_c_program(ParsedInputFileContents, ParsedQuery);
  } else if (NULL != mdl) {
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

    if (TYM_CONVERT_TO_SMT == Params->function) {
      printf("%s", tym_buffer_contents(outbuf));
    } else if (TYM_CONVERT_TO_SMT_AND_SOLVE == Params->function) {
#ifdef TYM_INTERFACE_Z3
      solver_loop(Params, &mdl, varmap, ParsedQuery, outbuf);
#else
      assert(0);
#endif // TYM_INTERFACE_Z3
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

  tym_free_atom_database(adb);

  // If we used a solver, check if it timed out or gave up,
  // so we can communicate this upwards through the return code.
  if (TYM_CONVERT_TO_SMT_AND_SOLVE == Params->function &&
      TYM_SAT_UNKNOWN == TymState_LastSolverResult) {
    return TYM_SOLVER_GAVEUP;
  } else {
    return TYM_AOK;
  }
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
