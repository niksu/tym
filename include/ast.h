/*
 * AST spec.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_AST_H__
#define __TYM_AST_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "buffer.h"
#include "util.h"

typedef enum {VAR=0, CONST=1, STR=2} term_kind_t;

struct term_t {
  term_kind_t kind;
  const char * identifier;
};

DECLARE_LIST_TYPE(terms_t, term, term_t)
DECLARE_LIST_MK(term, struct term_t, struct terms_t, /*no const*/)

struct atom_t {
  char * predicate;
  uint8_t arity;
  struct term_t * args;
};

DECLARE_LIST_TYPE(atoms_t, atom, atom_t)
DECLARE_LIST_MK(atom, struct atom_t, struct atoms_t, /*no const*/)

struct clause_t {
  struct atom_t head;
  uint8_t body_size;
  struct atom_t * body;
};

DECLARE_LIST_TYPE(clauses_t, clause, clause_t)
DECLARE_LIST_MK(clause, struct clause_t, struct clauses_t, /*no const*/)

struct program_t {
  uint8_t no_clauses;
  const struct clause_t ** program;
};

size_t my_strcpy(char * dst, const char * src, size_t * space);
size_t term_to_str(const struct term_t * const term, size_t * outbuf_size, char * outbuf);
size_t terms_to_str(const struct terms_t * const terms, size_t * outbuf_size, char * outbuf);
size_t predicate_to_str(const struct atom_t * atom, size_t * outbuf_size, char * outbuf);
size_t atom_to_str(const struct atom_t * const atom, size_t * outbuf_size, char * outbuf);
size_t clause_to_str(const struct clause_t * const clause, size_t * outbuf_size, char * outbuf);
size_t program_to_str(const struct program_t * const clause, size_t * outbuf_size, char * outbuf);

// FIXME move buf_strcpy to "buffer" module.
struct buffer_write_result * buf_strcpy(struct buffer_info * dst, const char * src);
struct buffer_write_result * Bterm_to_str(const struct term_t * const term, struct buffer_info * dst);
struct buffer_write_result * Bterms_to_str(const struct terms_t * const terms, struct buffer_info * dst);
struct buffer_write_result * Bpredicate_to_str(const struct atom_t * atom, struct buffer_info * dst);
struct buffer_write_result * Batom_to_str(const struct atom_t * const atom, struct buffer_info * dst);
struct buffer_write_result * Bclause_to_str(const struct clause_t * const clause, struct buffer_info * dst);
struct buffer_write_result * Bprogram_to_str(const struct program_t * const program, struct buffer_info * dst);

struct term_t * mk_const(const char * identifier);
struct term_t * mk_var(const char * identifier);
struct term_t * mk_term(term_kind_t kind, const char * identifier);
struct atom_t * mk_atom(char * predicate, uint8_t arity, const struct terms_t * args);
struct clause_t * mk_clause(struct atom_t * head, uint8_t body_size, const struct atoms_t * body);
struct program_t * mk_program(uint8_t no_clauses, const struct clauses_t * program);

DECLARE_U8_LIST_LEN(terms)
DECLARE_U8_LIST_LEN(atoms)
DECLARE_U8_LIST_LEN(clauses)

void free_term(struct term_t term);
void free_terms(struct terms_t * terms);
void free_atom(struct atom_t atom);
void free_atoms(struct atoms_t * atoms);
void free_clause(struct clause_t clause);
void free_clauses(struct clauses_t * clauses);
void free_program(struct program_t * program);

typedef int (*x_to_str_t)(void *, size_t * outbuf_size, char * outbuf);

void debug_out_syntax(void * x, int (*x_to_str)(void *, size_t * outbuf_size, char * outbuf));
void Bdebug_out_syntax(void * x, struct buffer_write_result * (*x_to_str)(void *, struct buffer_info * dst));

#if DEBUG
#define DBG_SYNTAX debug_out_syntax
#else
#define DBG_SYNTAX(...)
#endif // DEBUG

char hash_str(const char * str);
char hash_term(struct term_t);
char hash_atom(struct atom_t);
char hash_clause(struct clause_t);

typedef enum {NO_ERROR, DIFF_KIND_SAME_IDENTIFIER, DIFF_IDENTIFIER_SAME_KIND} eq_term_error_t;
bool eq_term(const struct term_t * const t1, const struct term_t * const t2, eq_term_error_t * error_code, bool * result);

struct term_t * copy_term(const struct term_t * const term);

bool terms_subsumed_by(const struct terms_t * const, const struct terms_t *);

#endif // __TYM_AST_H__
