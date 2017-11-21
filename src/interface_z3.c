/*
 * Translation from formulas to Z3's API.
 * Nik Sultana, April 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifdef TYM_INTERFACE_Z3
#include "interface_z3.h"

#include "stdlib.h"
#include "string.h"

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

  unsigned c = Z3_model_get_num_consts(z3_ctxt, z3_mdl);
  printf("Num consts: %d\n", c);
  for (unsigned i = 0; i < c; i++) {
    Z3_func_decl d = Z3_model_get_const_decl(z3_ctxt, z3_mdl, i);
    Z3_ast_opt a = Z3_model_get_const_interp(z3_ctxt, z3_mdl, d);
    assert(NULL != a);

    Z3_symbol symb = Z3_get_decl_name(z3_ctxt, d);
    char const * s = Z3_get_symbol_string(z3_ctxt, symb);
    printf("  symb: %s\n    ", s);

    for (unsigned j = 0; j < c; j++) {
      Z3_func_decl d2 = Z3_model_get_const_decl(z3_ctxt, z3_mdl, j);
      Z3_ast_opt a2 = Z3_model_get_const_interp(z3_ctxt, z3_mdl, d2);
      if (a2 == a) {
        Z3_symbol symb2 = Z3_get_decl_name(z3_ctxt, d2);
        char const * s2 = Z3_get_symbol_string(z3_ctxt, symb2);
        printf("%s ", s2);
      }
    }
    printf("\n");

    Z3_bool b = Z3_model_has_interp(z3_ctxt, z3_mdl, d);
    assert(b == Z3_TRUE);
  }

  if (NULL != z3_mdl) {
    Z3_model_dec_ref(z3_ctxt, z3_mdl);
  }
}

void
tym_z3_get_model(struct TymMdlValuations * vals)
{
  assert(NULL != vals);
  z3_mdl = Z3_solver_get_model(z3_ctxt, z3_slvr);
  if (NULL != z3_mdl) {
    Z3_model_inc_ref(z3_ctxt, z3_mdl);
  }

  unsigned c = Z3_model_get_num_consts(z3_ctxt, z3_mdl);
  for (unsigned i = 0; i < c; i++) {
    Z3_func_decl d = Z3_model_get_const_decl(z3_ctxt, z3_mdl, i);
    Z3_ast_opt a = Z3_model_get_const_interp(z3_ctxt, z3_mdl, d);
    assert(NULL != a);

    Z3_symbol symb = Z3_get_decl_name(z3_ctxt, d);
    const TymStr * s = TYM_CSTR_DUPLICATE(Z3_get_symbol_string(z3_ctxt, symb));

    for (unsigned vi = 0; vi < vals->count; vi++) {
      if (0 == tym_cmp_str(vals->v[vi].const_name, s)) {
        assert(NULL == vals->v[vi].value);

        for (unsigned j = 0; j < c; j++) {
          Z3_func_decl d2 = Z3_model_get_const_decl(z3_ctxt, z3_mdl, j);
          Z3_ast_opt a2 = Z3_model_get_const_interp(z3_ctxt, z3_mdl, d2);
          if (a2 == a) {
            Z3_symbol symb2 = Z3_get_decl_name(z3_ctxt, d2);
            const TymStr * s2 = TYM_CSTR_DUPLICATE(Z3_get_symbol_string(z3_ctxt, symb2));
            if (0 != tym_cmp_str(vals->v[vi].const_name, s2)) {
              vals->v[vi].value = s2;
            } else {
              tym_free_str(s2);
            }
          }
        }

      }
    }
    tym_free_str(s);
  }

  if (NULL != z3_mdl) {
    Z3_model_dec_ref(z3_ctxt, z3_mdl);
  }
}

#else
void tym_no_z3(void);
#endif // TYM_INTERFACE_Z3
