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
// FIXME const TymStr * tym_csyntax_program(const struct TymProgram *);
const TymStr * tym_csyntax_malloc(const struct TymCSyntax * csyn);
void tym_csyntax_free(const struct TymCSyntax * csyn);

const TymStr * tym_array_of(struct TymSymGen * namegen, const TymStr ** result_name, size_t array_size, const TymStr * expression_type, const TymStr ** expression_strs);

#endif // TYM_INTERFACE_C_H
