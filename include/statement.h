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

struct stmt_t {
  enum TymStmtKind kind;
  union {
    const struct TymFmla * axiom;
    struct TymStmtConst * const_def;
  } param;
};

struct TymUniverse * mk_universe(struct TymTerms *);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * universe_str(const struct TymUniverse * const, struct TymBufferInfo * dst);
void free_universe(struct TymUniverse *);

const struct stmt_t * mk_stmt_axiom(const struct TymFmla * axiom);
const struct stmt_t * mk_stmt_pred(char * pred_name, struct TymTerms * params, struct TymFmla * body);
struct stmt_t * mk_stmt_const(char * const_name, struct TymUniverse *, char * ty);
const struct stmt_t * mk_stmt_const_def(char * const_name, struct TymUniverse * uni);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * stmt_str(const struct stmt_t * const, struct TymBufferInfo * dst);
void free_stmt(const struct stmt_t *);

TYM_DECLARE_LIST_TYPE(stmts_t, stmt, stmt_t)
TYM_DECLARE_LIST_MK(stmt, struct stmt_t, struct stmts_t, const)
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * stmts_str(const struct stmts_t * const, struct TymBufferInfo * dst);
void free_stmts(const struct stmts_t *);

TYM_DECLARE_LIST_REV(stmts, const, struct stmts_t, const)

struct model_t {
  struct TymUniverse * universe;
  const struct stmts_t * stmts;
};

struct model_t * mk_model(struct TymUniverse *);
struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * model_str(const struct model_t * const, struct TymBufferInfo * dst);
void free_model(const struct model_t *);
void strengthen_model(struct model_t *, const struct stmt_t *);

struct TymTerm * new_const_in_stmt(const struct stmt_t * stmt);
struct TymTerms * consts_in_stmt(const struct stmt_t * stmt);

void statementise_universe(struct model_t * mdl);

#endif /* __TYM_STATEMENT_H__ */
