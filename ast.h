/*
 * AST spec.
 * TYM Datalog.
 * Nik Sultana, March 2017.
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
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

struct program_t {
  uint8_t no_clauses;
  struct clause_t ** program;
};

int my_strcpy(char * dst, const char * src, size_t * space);
int atom_to_str(struct atom_t* atom, size_t * outbuf_size, char* outbuf);
int clause_to_str(struct clause_t* clause, size_t * outbuf_size, char* outbuf);
int program_to_str(struct program_t* clause, size_t * outbuf_size, char* outbuf);
struct term_t * mk_term(term_kind_t kind, char * identifier);
struct atom_t * mk_atom(char * predicate, uint8_t arity, struct term_t ** rev_args);
struct clause_t * mk_clause(struct atom_t * head, uint8_t body_size, struct atom_t ** rev_body);
struct program_t * mk_program(uint8_t no_clauses, struct clause_t ** program);
