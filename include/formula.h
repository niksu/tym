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

extern uint8_t max_var_width;

static const char * const universe_ty = "Universe";

struct fmla_atom_t {
  char * pred_name;
  uint8_t arity;
  struct TymTerm ** predargs;
};

enum fmla_kind_t {FMLA_ATOM, FMLA_AND, FMLA_OR, FMLA_NOT, FMLA_EX, FMLA_CONST};

struct fmla_quant_t {
  const char * bv;
  struct fmla_t * body;
};

struct fmla_t {
  enum fmla_kind_t kind;
  union {
    bool const_value;
    struct fmla_atom_t * atom;
    struct fmla_t ** args;
    struct fmla_quant_t * quant;
  } param;
};

TYM_DECLARE_MUTABLE_LIST_TYPE(fmlas_t, fmla, fmla_t)
TYM_DECLARE_MUTABLE_LIST_MK(fmla, struct fmla_t, struct fmlas_t)

struct fmla_t * mk_fmla_const(bool b);
struct fmla_t * mk_fmla_atom(char * pred_name, uint8_t arity, struct TymTerm ** predargs);
struct fmla_t * mk_fmla_atom_varargs(char * pred_name, uint8_t arity, ...);
struct fmla_t * mk_fmla_quant(const char * bv, struct fmla_t * body);
struct fmla_t * mk_fmla_quants(const struct TymTerms * const vars, struct fmla_t * body);
struct fmla_t * mk_fmla_not(struct fmla_t * subfmla);
struct fmla_t * mk_fmla_and(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR);
struct fmla_t * mk_fmla_or(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR);
struct fmla_t * mk_fmla_ands(struct fmlas_t * fmlas);
struct fmla_t * mk_fmla_ors(struct fmlas_t * fmlas);
struct fmla_t * mk_fmla_imply(struct fmla_t * antecedent, struct fmla_t * consequent);
struct fmla_t * copy_fmla(const struct fmla_t * const);

struct fmlas_t * mk_fmlas(uint8_t no_fmlas, ...);

struct buffer_write_result * fmla_atom_str(struct fmla_atom_t * at, struct TymBufferInfo * dst);
struct buffer_write_result * fmla_quant_str(struct fmla_quant_t * quant, struct TymBufferInfo * dst);
struct buffer_write_result * fmla_str(const struct fmla_t * fmla, struct TymBufferInfo * dst);

struct sym_gen_t {
  const char * prefix;
  size_t index;
};

struct sym_gen_t * mk_sym_gen(const char * prefix);
struct sym_gen_t * copy_sym_gen(const struct sym_gen_t * const cp_orig);
char * mk_new_var(struct sym_gen_t *);

struct valuation_t {
  char * var;
  struct TymTerm * val;
  struct valuation_t * next;
};

size_t valuation_len(const struct valuation_t * const v);
struct buffer_write_result * valuation_str(struct valuation_t * v, struct TymBufferInfo * dst);

bool fmla_is_atom(const struct fmla_t * fmla);
struct fmla_atom_t * fmla_as_atom(const struct fmla_t * fmla);
const struct fmla_t * mk_abstract_vars(const struct fmla_t *, struct sym_gen_t *, struct valuation_t **);
struct TymTerms * arguments_of_atom(struct fmla_atom_t * fmla);

void free_fmla_atom(struct fmla_atom_t *);
void free_fmla_quant(struct fmla_quant_t *);
void free_fmla(const struct fmla_t *);
void free_fmlas(const struct fmlas_t *);
void free_sym_gen(struct sym_gen_t *);
void free_valuation(struct valuation_t *);

struct TymTerms * filter_var_values(struct valuation_t * const v);

size_t fmla_size(const struct fmla_t * const);

struct TymTerms * consts_in_fmla(const struct fmla_t * fmla, struct TymTerms * acc);

#endif /* __TYM_FORMULA_H__ */
