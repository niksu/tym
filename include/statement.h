/*
 * Representation of statements (that affect logical models).
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_STATEMENT_H__
#define __TYM_STATEMENT_H__

// NOTE only interested in finite models
struct TymUniverse {
  uint8_t cardinality;
  char ** element;
};

extern char * tym_bool_ty;
extern char * tym_distinctK;
extern char * tym_eqK;

struct TymStmtConst {
  char * const_name;
  struct TymTerms * params;
  struct TymFmla * body;
  char * ty;
};

enum TymStmtKind {TYM_STMT_AXIOM, TYM_STMT_CONST_DEF};

struct TymStmt {
  enum TymStmtKind kind;
  union {
    const struct TymFmla * axiom;
    struct TymStmtConst * const_def;
  } param;
};

struct TymUniverse * mk_universe(struct TymTerms *);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * universe_str(const struct TymUniverse * const, struct TymBufferInfo * dst);
void free_universe(struct TymUniverse *);

const struct TymStmt * mk_stmt_axiom(const struct TymFmla * axiom);
const struct TymStmt * mk_stmt_pred(char * pred_name, struct TymTerms * params, struct TymFmla * body);
struct TymStmt * mk_stmt_const(char * const_name, struct TymUniverse *, char * ty);
const struct TymStmt * mk_stmt_const_def(char * const_name, struct TymUniverse * uni);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * stmt_str(const struct TymStmt * const, struct TymBufferInfo * dst);
void free_stmt(const struct TymStmt *);

TYM_DECLARE_LIST_TYPE(TymStmts, stmt, TymStmt)
TYM_DECLARE_LIST_MK(stmt, struct TymStmt, struct TymStmts, const)
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * stmts_str(const struct TymStmts * const, struct TymBufferInfo * dst);
void free_stmts(const struct TymStmts *);

TYM_DECLARE_LIST_REV(stmts, const, struct TymStmts, const)

struct TymModel {
  struct TymUniverse * universe;
  const struct TymStmts * stmts;
};

struct TymModel * mk_model(struct TymUniverse *);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * model_str(const struct TymModel * const, struct TymBufferInfo * dst);
void free_model(const struct TymModel *);
void strengthen_model(struct TymModel *, const struct TymStmt *);

struct TymTerm * new_const_in_stmt(const struct TymStmt * stmt);
struct TymTerms * consts_in_stmt(const struct TymStmt * stmt);

void statementise_universe(struct TymModel * mdl);

#endif /* __TYM_STATEMENT_H__ */
