/*
 * Translation from formulas to Z3's API.
 * Nik Sultana, April 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifdef INTERFACE_Z3
#include "interface_z3.h"

static Z3_sort universe_sort = NULL;
static Z3_context z3_ctxt = NULL;
static Z3_solver z3_slvr = NULL;
static Z3_model z3_mdl = NULL;
static Z3_lbool z3_result = Z3_L_UNDEF;

void
tym_z3_begin(void)
{
  if ((NULL != universe_sort) || (NULL != z3_ctxt)) {
    // FIXME complain
  } else {
    Z3_config cfg = Z3_mk_config();
    Z3_set_param_value(cfg, "model", "true");
    z3_ctxt = Z3_mk_context(cfg);
    Z3_del_config(cfg);

    Z3_symbol universe_sort_name = Z3_mk_string_symbol(z3_ctxt, "Universe");
    universe_sort = Z3_mk_uninterpreted_sort(z3_ctxt, universe_sort_name);

    z3_slvr = Z3_mk_solver(z3_ctxt);
    Z3_solver_inc_ref(z3_ctxt, z3_slvr);
  }
}

void
tym_z3_end(void)
{
  Z3_solver_dec_ref(z3_ctxt, z3_slvr);
  Z3_del_context(z3_ctxt);
}

enum TymSatisfiable
tym_z3_satisfied(void)
{
  switch (z3_result) {
  case Z3_L_UNDEF:
    return TYM_SAT_UNKNOWN;
  case Z3_L_TRUE:
    return TYM_SAT_YES;
  case Z3_L_FALSE:
    return TYM_SAT_NO;
  default:
    assert(0);
  }
}

void
tym_z3_check(void)
{
  z3_result = Z3_solver_check(z3_ctxt, z3_slvr);
}

void
tym_z3_assert_smtlib2(const char * str)
{
  assert(NULL != z3_ctxt);
  Z3_ast fmla = Z3_parse_smtlib2_string(z3_ctxt,
      str, 0,
      NULL,
      NULL,
      0,
      NULL,
      NULL);
  Z3_solver_assert(z3_ctxt, z3_slvr, fmla);
}

void
tym_z3_print_model(void)
{
  // FIXME Display only interpretations of constants appearing in the query.
  z3_mdl = Z3_solver_get_model(z3_ctxt, z3_slvr);
  if (NULL != z3_mdl) {
    Z3_model_inc_ref(z3_ctxt, z3_mdl);
    printf("z3_mdl:\n%s\n", Z3_model_to_string(z3_ctxt, z3_mdl));
  }

  if (NULL != z3_mdl) {
    Z3_model_dec_ref(z3_ctxt, z3_mdl);
  }
}

//    Z3_ast Z3_API Z3_parse_smtlib2_string(Z3_context c,
//    void Z3_API Z3_parse_smtlib_string(Z3_context c,
//
//    void Z3_API Z3_get_version(unsigned * major, unsigned * minor, unsigned * build_number, unsigned * revision_number);
//    void Z3_API Z3_finalize_memory(void);
//    void Z3_API Z3_goal_assert(Z3_context c, Z3_goal g, Z3_ast a);
//#ifndef SAFE_ERRORS

//z3_begin(struct model_t * mdl)

//return Z3_mk_const(ctx, s, ty);


//struct stmt_t *
//struct stmts_t *
//struct fmla_t *
//struct fmla_t *
#else
void tym_no_z3(void);
#endif // INTERFACE_Z3
