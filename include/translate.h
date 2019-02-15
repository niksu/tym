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


This file: Translation between clause representations.
*/

#ifndef TYM_TRANSLATE_H
#define TYM_TRANSLATE_H

#include <assert.h>

#include "ast.h"
#include "formula.h"
#include "statement.h"
#include "symbols.h"

struct TymFmla * tym_translate_atom(const struct TymAtom * at);

struct TymFmla * tym_translate_body(const struct TymClause * cl);

struct TymFmlas * tym_translate_bodies(const struct TymClauses * cls);

struct TymFmla * tym_translate_valuation(struct TymValuation * const v);

void tym_translate_query_fmla_atom(struct TymModel * mdl, struct TymSymGen * cg, struct TymFmlaAtom * at, struct TymValuation ** varmap);
void tym_translate_query_fmla(struct TymModel * mdl, struct TymSymGen * cg, struct TymFmla * fmla, struct TymValuation ** varmap);

struct TymValuation * tym_translate_query(struct TymProgram * query, struct TymModel * mdl, struct TymSymGen * cg);

struct TymModel * tym_translate_program(struct TymProgram * program, struct TymSymGen ** vg, struct TymAtomDatabase * adb);

struct TymStmts * tym_order_statements(struct TymStmts * stmts);

#endif /* TYM_TRANSLATE_H */
