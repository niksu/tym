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


This file: Translating parsed Datalog into C code.
*/

#ifndef TYM_INTERFACE_C_H
#define TYM_INTERFACE_C_H

#include "ast.h"
#include "formula.h"
#include "string_idx.h"
#include "util.h"

enum TymSyntaxKind {TYM_TERM, TYM_ATOM, TYM_CLAUSE, TYM_PROGRAM};

struct TymCSyntax {
  const TymStr * type;
  const TymStr * name;
  const TymStr * serialised;
  enum TymSyntaxKind kind;
  const void * original;
};

const struct TymCSyntax * tym_csyntax_term(struct TymSymGen * namegen, const struct TymTerm * term);
const struct TymCSyntax * tym_csyntax_atom(struct TymSymGen * namegen, const struct TymAtom *);
const struct TymCSyntax * tym_csyntax_clause(struct TymSymGen * namegen, const struct TymClause *);
const struct TymCSyntax * tym_csyntax_program(struct TymSymGen * namegen, const struct TymProgram *);
const TymStr * tym_csyntax_malloc(const struct TymCSyntax * csyn);
const TymStr * tym_csyntax_address_of(const struct TymCSyntax * csyn);
void tym_csyntax_free(const struct TymCSyntax * csyn);

const TymStr * tym_array_of(struct TymSymGen * namegen, const TymStr ** result_name, size_t array_size, const TymStr * expression_type, const TymStr ** expression_strs);

#endif // TYM_INTERFACE_C_H
