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

struct fmla_t * translate_atom(struct atom_t * at);

struct fmla_t * translate_body(struct clause_t * cl);

struct fmlas_t * translate_bodies(struct clauses_t * cls);

struct fmla_t * translate_valuation(struct valuation_t * const v);

struct fmla_t * translate_query_fmla_atom(struct model_t * mdl, struct sym_gen_t * cg, const struct fmla_atom_t * at);

struct fmla_t * translate_query_fmla(struct model_t * mdl, struct sym_gen_t * cg, const struct fmla_t * fmla);

void translate_query(struct program_t * query, struct model_t * mdl, struct sym_gen_t * cg);

#endif /* __TYM_TRANSLATE_H__ */
