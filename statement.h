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

struct stmt_const_t {
  const char * const_name;
  struct terms_t * params;
  struct fmla_t * body;
};

enum stmt_kind_t {STMT_AXIOM, STMT_CONST_DEF};

struct stmt_t {
  enum stmt_kind_t kind;
  union {
    struct fmla_t * axiom;
    struct stmt_const_t * const_def;
  } param;
};

struct universe_t * mk_universe(struct terms_t *);
size_t universe_str(struct universe_t *, size_t * remaining, char * buf);
void free_universe(struct universe_t *);

struct stmt_t * mk_stmt_axiom(struct fmla_t * axiom);
struct stmt_t * mk_stmt_pred(char * pred_name, struct terms_t * params, struct fmla_t * body);
struct stmt_t * mk_stmt_const(char * const_name, struct universe_t *);
size_t stmt_str(struct stmt_t *, size_t * remaining, char * buf);
void free_stmt(struct stmt_t *);

struct stmts_t {
  struct stmt_t * stmt;
  struct stmts_t * next;
};

struct stmts_t * mk_stmt_cell(struct stmt_t * stmt, struct stmts_t * next);
size_t stmts_str(struct stmts_t *, size_t * remaining, char * buf);
void free_stmts(struct stmts_t *);

struct model_t {
  struct universe_t universe;
  struct stmts_t stmts;
};

struct model_t * mk_model(struct universe_t *);
size_t model_str(struct model_t *, size_t * remaining, char * buf);
void free_model(struct model_t *);
void strengthen_model(struct model_t *, struct stmt_t *);

#endif /* __TYM_STATEMENT_H__ */
