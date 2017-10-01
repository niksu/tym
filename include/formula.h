/*
 * Representation of formulas.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_FORMULA_H__
#define __TYM_FORMULA_H__

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_idx.h"

extern uint8_t TymMaxVarWidth;
extern char * TYM_UNIVERSE_TY;

struct TymFmlaAtom {
  const TymStr * pred_name;
  const struct TymTerm * pred_const;
  uint8_t arity;
  struct TymTerm ** predargs;
};

enum TymFmlaKind {FMLA_ATOM, FMLA_AND, FMLA_OR, FMLA_NOT, FMLA_EX, FMLA_CONST};

struct TymFmlaQuant {
  const TymStr * bv;
  struct TymFmla * body;
};

struct TymFmla {
  enum TymFmlaKind kind;
  union {
    bool const_value;
    struct TymFmlaAtom * atom;
    struct TymFmla ** args;
    struct TymFmlaQuant * quant;
  } param;
};

TYM_DECLARE_MUTABLE_LIST_TYPE(TymFmlas, fmla, TymFmla)
TYM_DECLARE_MUTABLE_LIST_MK(fmla, struct TymFmla, struct TymFmlas)
TYM_DECLARE_LIST_LEN(TymFmlas, , struct TymFmlas)

struct TymFmla * tym_mk_fmla_const(bool b);
struct TymFmla * tym_mk_fmla_atom(const TymStr * pred_name, uint8_t arity, struct TymTerm ** predargs);
struct TymFmla * tym_mk_fmla_atom_varargs(const TymStr * pred_name, unsigned int arity, ...);
struct TymFmla * tym_mk_fmla_quant(const TymStr * bv, struct TymFmla * body);
struct TymFmla * tym_mk_fmla_quants(const struct TymTerms * const vars, struct TymFmla * body);
struct TymFmla * tym_mk_fmla_not(struct TymFmla * subfmla);
struct TymFmla * tym_mk_fmla_and(struct TymFmla * subfmlaL, struct TymFmla * subfmlaR);
struct TymFmla * tym_mk_fmla_or(struct TymFmla * subfmlaL, struct TymFmla * subfmlaR);
struct TymFmla * tym_mk_fmla_ands(struct TymFmlas * fmlas);
struct TymFmla * tym_mk_fmla_ors(struct TymFmlas * fmlas);
struct TymFmla * tym_mk_fmla_imply(struct TymFmla * antecedent, struct TymFmla * consequent);
struct TymFmla * tym_copy_fmla(const struct TymFmla * const);

struct TymFmlas * tym_mk_fmlas(unsigned int no_fmlas, ...);

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_fmla_atom_str(struct TymFmlaAtom * at, struct TymBufferInfo * dst);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_fmla_quant_str(struct TymFmlaQuant * quant, struct TymBufferInfo * dst);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_fmla_str(const struct TymFmla * fmla, struct TymBufferInfo * dst);

struct TymSymGen {
  const TymStr * prefix;
  size_t index;
};

struct TymSymGen * tym_mk_sym_gen(const TymStr * prefix);
struct TymSymGen * tym_copy_sym_gen(const struct TymSymGen * const cp_orig);
const TymStr * tym_mk_new_var(struct TymSymGen *);

struct TymValuation {
  const TymStr * var;
  struct TymTerm * val;
  struct TymValuation * next;
};

size_t tym_valuation_len(const struct TymValuation * const v);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_valuation_str(struct TymValuation * v, struct TymBufferInfo * dst);

bool tym_fmla_is_atom(const struct TymFmla * fmla);
struct TymFmlaAtom * tym_fmla_as_atom(const struct TymFmla * fmla);
bool tym_fmla_is_const(const struct TymFmla * fmla);
bool tym_fmla_as_const(const struct TymFmla * fmla);
const struct TymFmla * tym_mk_abstract_vars(const struct TymFmla *, struct TymSymGen *, struct TymValuation **);
struct TymTerms * tym_arguments_of_atom(struct TymFmlaAtom * fmla);

void tym_free_fmla_atom(struct TymFmlaAtom *);
void tym_free_fmla_quant(struct TymFmlaQuant *);
void tym_free_fmla(const struct TymFmla *);
void tym_free_fmlas(const struct TymFmlas *);
void tym_free_sym_gen(struct TymSymGen *);
void tym_free_valuation(struct TymValuation *);

struct TymTerms * tym_filter_var_values(struct TymValuation * const v);

size_t tym_fmla_size(const struct TymFmla * const);

struct TymTerms * tym_consts_in_fmla(const struct TymFmla * fmla, struct TymTerms * acc, bool with_pred_const);

#endif /* __TYM_FORMULA_H__ */
