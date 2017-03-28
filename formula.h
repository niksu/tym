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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fmla_atom_t {
  char * pred_name;
  uint8_t arity;
  char ** predargs;
};

enum fmla_kind_t {FMLA_ATOM, FMLA_AND, FMLA_OR, FMLA_NOT, FMLA_ALL, FMLA_CONST};

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

struct fmla_t * mk_fmla_const(bool b);
struct fmla_t * mk_fmla_atom(char * pred_name, uint8_t arity, char ** args);
struct fmla_t * mk_fmla_quant(const char * bv, struct fmla_t * body);
struct fmla_t * mk_fmla_not(struct fmla_t * subfmla);
struct fmla_t * mk_fmla_and(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR);
struct fmla_t * mk_fmla_or(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR);
struct fmla_t * mk_fmla_imply(struct fmla_t * antecedent, struct fmla_t * consequent);

size_t fmla_atom_str(struct fmla_atom_t * at, size_t * remaining, char * buf);
size_t fmla_quant_str(struct fmla_quant_t * at, size_t * remaining, char * buf);
size_t fmla_str(struct fmla_t * at, size_t * remaining, char * buf);

struct fmlas_t {
  struct fmla_t * fmla;
  struct fmlas_t * next;
};

struct fmlas_t * mk_fmla_cell(struct fmla_t * fmla, struct fmlas_t * next);

struct var_gen_t {
  const char * prefix;
  size_t index;
};

struct var_gen_t * mk_var_gen(const char * prefix);
char * mk_new_var(struct var_gen_t *);

struct valuation_t {
  char * var;
  char * val;
  struct valuation_t * next;
};

size_t valuation_str(struct valuation_t * v, size_t * remaining, char * buf);

bool fmla_is_atom(struct fmla_t * fmla);
struct fmla_atom_t * fmla_as_atom(struct fmla_t * fmla);
struct fmla_t * mk_abstract_vars(struct fmla_t *, struct var_gen_t *, struct valuation_t **);

#endif /* __TYM_FORMULA_H__ */
