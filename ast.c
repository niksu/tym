/*
 * AST supporting functions.
 * TYM Datalog.
 * Nik Sultana, March 2017.
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <stdlib.h>
#include <string.h>
#include "ast.h"

int my_strcpy(char * dst, const char * src, size_t * space) {
  int l = strlen(src);
  if (l < *space) {
    strcpy(dst, src);
  } else {
    return -1;
  }

  *space -= l;
  return l;
}

int term_to_str(struct term_t* term, size_t * outbuf_size, char* outbuf) {
  int l = my_strcpy(outbuf, term->identifier, outbuf_size);
  if (l < 0) {
    // FIXME complain
  }

  return l;
}

int predicate_to_str(struct atom_t* atom, size_t * outbuf_size, char* outbuf) {
  int l = my_strcpy(outbuf, atom->predicate, outbuf_size);
  if (l < 0) {
    // FIXME complain
  }

  return l;
}

int atom_to_str(struct atom_t* atom, size_t * outbuf_size, char* outbuf) {
  int l = predicate_to_str(atom, outbuf_size, outbuf);
  if (l < 0) {
    // FIXME complain
    return l;
  }

  // There needs to be space to store at least "()\0".
  if (*outbuf_size < 3) {
    // FIXME complain
    return -1;
  }

  outbuf[(*outbuf_size)--, l++] = '(';

  for (int i = 0; i < atom->arity; i++) {
    int l_sub = term_to_str(&(atom->args[i]), outbuf_size, outbuf + l);
    if (l_sub < 0) {
      // FIXME complain
      return l_sub;
    }

    l += l_sub;

    if (i != atom->arity - 1) {
      // There needs to be space to store at least ", x)\0".
      if (*outbuf_size < 5) {
        // FIXME complain
        return -1;
      }

      // FIXME batch these.
      outbuf[(*outbuf_size)--, l++] = ',';
      outbuf[(*outbuf_size)--, l++] = ' ';
    }
  }

  // FIXME batch these.
  outbuf[(*outbuf_size)--, l++] = ')';
  outbuf[(*outbuf_size)--, l++] = '\0';

  return l;
}

int clause_to_str(struct clause_t* clause, size_t * outbuf_size, char* outbuf) {
  int l = atom_to_str(&(clause->head), outbuf_size, outbuf);
  if (l < 0) {
    // FIXME complain
    return l;
  }

  (*outbuf_size)++, l--; // chomp the trailing \0.

  if (clause->body_size > 0) {
    // There needs to be space to store at least " :- x().".
    if (*outbuf_size < 8) {
      // FIXME complain
      return -1;
    }

    // FIXME batch these.
    outbuf[(*outbuf_size)--, l++] = ' ';
    outbuf[(*outbuf_size)--, l++] = ':';
    outbuf[(*outbuf_size)--, l++] = '-';
    outbuf[(*outbuf_size)--, l++] = ' ';

    for (int i = 0; i < clause->body_size; i++) {
      int l_sub = atom_to_str(&(clause->body[i]), outbuf_size, outbuf + l);

      if (l_sub < 0) {
        // FIXME complain
        return l_sub;
      }

      (*outbuf_size)++, l--; // chomp the trailing \0.

      l += l_sub;

      if (i != clause->body_size - 1) {
        // There needs to be space to store at least ", x().\0".
        if (*outbuf_size < 7) {
          // FIXME complain
          return -1;
        }

        // FIXME batch these.
        outbuf[(*outbuf_size)--, l++] = ',';
        outbuf[(*outbuf_size)--, l++] = ' ';
      }
    }
  }

  // There needs to be space to store at least ".\0".
  if (*outbuf_size < 2) {
    // FIXME complain
    return -1;
  }

  // FIXME batch these.
  outbuf[(*outbuf_size)--, l++] = '.';
  outbuf[(*outbuf_size)--, l++] = '\0';

  return l;
}

struct term_t * mk_term(term_kind_t kind, char * identifier) {
  struct term_t * t = (struct term_t *)malloc(sizeof(struct term_t));
  t->kind = kind;
  t->identifier = identifier;
  return t;
}

struct atom_t * mk_atom(char * predicate, uint8_t arity, struct term_t ** rev_args) {
  struct atom_t * at = (struct atom_t *)malloc(sizeof(struct atom_t));
  at->predicate = predicate;
  at->arity = arity;

  if (at->arity > 0) {
    at->args = (struct term_t *)malloc(sizeof(struct term_t) * at->arity);
    for (int i = 0; i < at->arity; i++) {
      at->args[i] = *(rev_args[--arity]);
    }
  }

  return at;
}

struct clause_t * mk_clause(struct atom_t * head, uint8_t body_size, struct atom_t ** rev_body) {
  struct clause_t * cl = (struct clause_t *)malloc(sizeof(struct clause_t));
  cl->head = *head;
  cl->body_size = body_size;

  if (cl->body_size > 0) {
    cl->body = (struct atom_t *)malloc(sizeof(struct atom_t) * cl->body_size);
    for (int i = 0; i < cl->body_size; i++) {
      cl->body[i] = *(rev_body[--body_size]);
    }
  }

  return cl;
}
