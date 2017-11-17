/*
 * Translation from formulas to Z3's API.
 * Nik Sultana, April 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef TYM_INTERFACE_Z3_H
#define TYM_INTERFACE_Z3_H

#include "string_idx.h"

#include "assert.h"
#include "z3.h"

enum TymSatisfiable {TYM_SAT_UNKNOWN=0, TYM_SAT_YES, TYM_SAT_NO};

void tym_z3_begin(void);
void tym_z3_end(void);
enum TymSatisfiable tym_z3_satisfied(void);
void tym_z3_check(void);
void tym_z3_assert_smtlib2(const char * str);

struct TymMdlValuation {
  const TymStr * name;
  const TymStr * value;
};

struct TymMdlValuations {
  unsigned count;
  struct TymMdlValuation * v;
};

struct TymMdlValuations * tym_z3_mk_valuations(const TymStr **);
void tym_z3_free_valuations(struct TymMdlValuations *);
void tym_z3_print_valuations(const struct TymMdlValuations *);
void tym_z3_reset_valuations(struct TymMdlValuations *);

void tym_z3_get_model(struct TymMdlValuations *);

void tym_z3_print_model(void);

#endif // TYM_INTERFACE_Z3_H
