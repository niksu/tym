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
struct universe_t {
  uint8_t cardinality;
  char ** element;
};

extern const char * const bool_ty;
extern const char * const distinctK;
extern const char * const eqK;

struct stmt_const_t {
  const char * const_name;
  struct TymTerms * params;
  const struct fmla_t * body;
  const char * const ty;
};

enum stmt_kind_t {STMT_AXIOM, STMT_CONST_DEF};

struct stmt_t {
  enum stmt_kind_t kind;
  union {
    const struct fmla_t * axiom;
    struct stmt_const_t * const_def;
  } param;
};

struct universe_t * mk_universe(struct TymTerms *);
struct buffer_write_result * universe_str(const struct universe_t * const, struct TymBufferInfo * dst);
void free_universe(struct universe_t *);

const struct stmt_t * mk_stmt_axiom(const struct fmla_t * axiom);
const struct stmt_t * mk_stmt_pred(char * pred_name, struct TymTerms * params, const struct fmla_t * body);
struct stmt_t * mk_stmt_const(char * const_name, struct universe_t *, const char * const ty);
const struct stmt_t * mk_stmt_const_def(char * const_name, struct universe_t * uni);
struct buffer_write_result * stmt_str(const struct stmt_t * const, struct TymBufferInfo * dst);
void free_stmt(const struct stmt_t *);

TYM_DECLARE_LIST_TYPE(stmts_t, stmt, stmt_t)
TYM_DECLARE_LIST_MK(stmt, struct stmt_t, struct stmts_t, const)
struct buffer_write_result * stmts_str(const struct stmts_t * const, struct TymBufferInfo * dst);
void free_stmts(const struct stmts_t *);

TYM_DECLARE_LIST_REV(stmts, const, struct stmts_t, const)

struct model_t {
  struct universe_t * universe;
  const struct stmts_t * stmts;
};

struct model_t * mk_model(struct universe_t *);
struct buffer_write_result * model_str(const struct model_t * const, struct TymBufferInfo * dst);
void free_model(const struct model_t *);
void strengthen_model(struct model_t *, const struct stmt_t *);

struct TymTerm * new_const_in_stmt(const struct stmt_t * stmt);
struct TymTerms * consts_in_stmt(const struct stmt_t * stmt);

void statementise_universe(struct model_t * mdl);

#endif /* __TYM_STATEMENT_H__ */
