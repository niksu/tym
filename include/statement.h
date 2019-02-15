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


This file: Representation of statements (that affect logical models).
*/

#ifndef TYM_STATEMENT_H
#define TYM_STATEMENT_H

#include "string_idx.h"

// NOTE only interested in finite models
struct TymUniverse {
  uint8_t cardinality;
  const TymStr ** element;
};

extern char * tym_bool_ty;
extern char * tym_distinctK;
extern char * tym_eqK;

struct TymStmtConst {
  const TymStr * const_name;
  struct TymTerms * params;
  struct TymFmla * body;
  const TymStr * ty;
};

enum TymStmtKind {TYM_STMT_AXIOM, TYM_STMT_CONST_DEF};

struct TymStmt {
  enum TymStmtKind kind;
  union {
    const struct TymFmla * axiom;
    struct TymStmtConst * const_def;
  } param;
};

struct TymUniverse * tym_mk_universe(struct TymTerms *);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_universe_str(const struct TymUniverse * const, struct TymBufferInfo * dst);
void tym_free_universe(struct TymUniverse *);

struct TymStmt * tym_mk_stmt_axiom(struct TymFmla * axiom);
struct TymStmt * tym_mk_stmt_pred(const TymStr * pred_name, struct TymTerms * params, struct TymFmla * body);
struct TymStmt * tym_split_stmt_pred(struct TymStmt * stmt);
struct TymStmt * tym_mk_stmt_const(const TymStr * const_name, struct TymUniverse *, const TymStr * ty);
struct TymStmt * tym_mk_stmt_const_def(const TymStr * const_name, struct TymUniverse * uni);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_stmt_str(const struct TymStmt * const, struct TymBufferInfo * dst);
void tym_free_stmt(const struct TymStmt *);

TYM_DECLARE_MUTABLE_LIST_TYPE(TymStmts, stmt, TymStmt)
TYM_DECLARE_MUTABLE_LIST_MK(stmt, struct TymStmt, struct TymStmts)
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_stmts_str(const struct TymStmts * const, struct TymBufferInfo * dst);
void tym_free_stmts(const struct TymStmts *);

TYM_DECLARE_LIST_REV(stmts, , struct TymStmts, )

struct TymModel {
  struct TymUniverse * universe;
  struct TymStmts * stmts;
};

struct TymModel * tym_mk_model(struct TymUniverse *);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_model_str(const struct TymModel * const, struct TymBufferInfo * dst);
void tym_free_model(const struct TymModel *);
void tym_strengthen_model(struct TymModel *, struct TymStmt *);

struct TymTerm * tym_new_const_in_stmt(const struct TymStmt * stmt);
struct TymTerms * tym_consts_in_stmt(const struct TymStmt * stmt);

void tym_statementise_universe(struct TymModel * mdl);

#endif /* __TYM_STATEMENT_H__ */
