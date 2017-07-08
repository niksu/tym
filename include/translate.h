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

struct TymFmla * translate_atom(const struct TymAtom * at);

struct TymFmla * translate_body(const struct TymClause * cl);

struct TymFmlas * translate_bodies(const struct TymClauses * cls);

struct TymFmla * translate_valuation(struct TymValuation * const v);

void translate_query_fmla_atom(struct model_t * mdl, struct TymSymGen * cg, struct TymFmlaAtom * at);
void translate_query_fmla(struct model_t * mdl, struct TymSymGen * cg, struct TymFmla * fmla);

void translate_query(struct TymProgram * query, struct model_t * mdl, struct TymSymGen * cg);

struct model_t * translate_program(struct TymProgram * program, struct TymSymGen ** vg);

const struct stmts_t * order_statements(const struct stmts_t * stmts);

#endif /* __TYM_TRANSLATE_H__ */
