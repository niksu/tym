/*
 * Translation between clause representations.
 * Nik Sultana, April 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_TRANSLATE_H__
#define __TYM_TRANSLATE_H__

#include <assert.h>

#include "ast.h"
#include "formula.h"
#include "statement.h"
#include "symbols.h"

struct fmla_t * translate_atom(const struct Atom * at);

struct fmla_t * translate_body(const struct Clause * cl);

struct fmlas_t * translate_bodies(const struct Clauses * cls);

struct fmla_t * translate_valuation(struct valuation_t * const v);

void translate_query_fmla_atom(struct model_t * mdl, struct sym_gen_t * cg, struct fmla_atom_t * at);
void translate_query_fmla(struct model_t * mdl, struct sym_gen_t * cg, struct fmla_t * fmla);

void translate_query(struct Program * query, struct model_t * mdl, struct sym_gen_t * cg);

struct model_t * translate_program(struct Program * program, struct sym_gen_t ** vg);

const struct stmts_t * order_statements(const struct stmts_t * stmts);

#endif /* __TYM_TRANSLATE_H__ */
