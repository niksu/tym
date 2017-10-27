/*
 * Representation of statements (that affect logical models).
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
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
