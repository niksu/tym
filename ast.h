/*
 * AST spec.
 * TYM Datalog.
 * Nik Sultana, March 2017.
 *
 * For licensing terms (LGPL) see the file called LICENSE.
 */

#include <stdlib.h>
#include <stdint.h>

typedef enum {Var, Const, Str} term_kind_t;

struct term_t {
  term_kind_t kind;
  char * identifier;
};

struct atom_t {
  char * predicate;
  uint8_t arity;
  struct term_t * args;
};

struct clause_t {
  struct atom_t head;
  uint8_t body_size;
  struct atom_t * body;
};
