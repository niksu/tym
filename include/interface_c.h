/*
 * Translating parsed Datalog into C code.
 * Nik Sultana, November 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef TYM_INTERFACE_C_H
#define TYM_INTERFACE_C_H

#include "string_idx.h"
#include "util.h"

void tym_csyntax_term(const TymStr ** definiens, const TymStr ** definition, struct TymSymGen * namegen, const struct TymTerm * term);
const TymStr * tym_csyntax_atom(const struct TymAtom *);
const TymStr * tym_csyntax_clause(const struct TymClause *);
const TymStr * tym_csyntax_program(const struct TymProgram *);

#endif // TYM_INTERFACE_C_H
