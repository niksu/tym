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


This file: Translating parsed Datalog into C code.
*/

#include "ast.h"
#include "formula.h"
#include "interface_c.h"
#include "module_tests.h"

const char * TymTermKindStr[] = {"TYM_VAR", "TYM_CONST", "TYM_STR"};

const struct TymCSyntax *
tym_csyntax_term(struct TymSymGen * namegen, const struct TymTerm * term)
{
  struct TymCSyntax * result = malloc(sizeof(*result));

  const char * identifier = tym_decode_str(term->identifier);
  char * str_buf = malloc(sizeof(*str_buf) * TYM_BUF_SIZE);

  result->name = tym_mk_new_var(namegen);
  result->type = TYM_CSTR_DUPLICATE("struct TymTerm");

  int buf_occupied = sprintf(str_buf, "%s %s = (%s){.kind = %s, .identifier = TYM_CSTR_DUPLICATE(\"%s\")};\n",
    tym_decode_str(result->type), tym_decode_str(result->name),
    tym_decode_str(result->type), TymTermKindStr[term->kind],
    identifier);
  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);

  char * trimmed_str_buf = malloc(sizeof(*trimmed_str_buf) * (1 + strlen(str_buf)));
  strcpy(trimmed_str_buf, str_buf);
  free(str_buf);
  result->serialised = tym_encode_str(trimmed_str_buf);
  result->kind = TYM_TERM;
  result->original = term;
  return result;
}

/* FIXME copied from ast.c */
void
tym_test_clause_csyn(void) {
  printf("***test_clause_syn***\n");
  struct TymTerm * t = malloc(sizeof *t);
  *t = (struct TymTerm){.kind = TYM_CONST,
    .identifier = TYM_CSTR_DUPLICATE("ok")};

  struct TymAtom * at = malloc(sizeof *at);
  at->predicate = TYM_CSTR_DUPLICATE("world");
  at->arity = 1;
  at->args = malloc(sizeof *at->args * 1);
  at->args[0] = tym_copy_term(t);

  struct TymAtom * hd = malloc(sizeof *hd);
  hd->predicate = TYM_CSTR_DUPLICATE("hello");
  hd->arity = 0;
  hd->args = NULL;

  struct TymClause * cl = malloc(sizeof *cl);
  cl->head = hd;
  cl->body_size = 1;
  cl->body = malloc(sizeof *cl->body * 1);
  cl->body[0] = tym_copy_atom(at); // Copying "at" since otherwise freeing "at" and "cl" would double-free "at".

  struct TymProgram * prog = malloc(sizeof *prog);
  prog->no_clauses = 1;
  prog->program = malloc(sizeof *prog->program * 1);
  prog->program[0] = cl;

  struct TymSymGen * sg = tym_mk_sym_gen(tym_encode_str(strdup("var")));
  const struct TymCSyntax * csyn_t = tym_csyntax_term(sg, t);
  const struct TymCSyntax * csyn_at = tym_csyntax_atom(sg, at);
  const struct TymCSyntax * csyn_cl = tym_csyntax_clause(sg, cl);
  const struct TymCSyntax * csyn_prog = tym_csyntax_program(sg, prog);

  printf("serialised: %s\n", tym_decode_str(csyn_t->serialised));
  printf("serialised: %s\n", tym_decode_str(csyn_at->serialised));
  printf("serialised: %s\n", tym_decode_str(csyn_cl->serialised));
  printf("serialised: %s\n", tym_decode_str(csyn_prog->serialised));
  const TymStr * malloc_str = tym_csyntax_malloc(csyn_t);
  printf("serialised: %s\n", tym_decode_str(malloc_str));

  tym_free_sym_gen(sg);
  tym_free_term(t);
  tym_free_atom(at);
  tym_free_program(prog); // prog references cl, so we don't call tym_free_clause(cl);
  tym_csyntax_free(csyn_t);
  tym_csyntax_free(csyn_at);
  tym_csyntax_free(csyn_cl);
  tym_csyntax_free(csyn_prog);
}

const TymStr *
tym_csyntax_malloc(const struct TymCSyntax * csyn)
{
  const TymStr * malloc_str_prefix = TYM_CSTR_DUPLICATE("malloc(sizeof(*");
  const TymStr * malloc_str_suffix = TYM_CSTR_DUPLICATE("))");
  return tym_append_str (malloc_str_prefix, tym_append_str (csyn->name, malloc_str_suffix));
}

const TymStr *
tym_csyntax_address_of(const struct TymCSyntax * csyn)
{
  const TymStr * str_prefix = TYM_CSTR_DUPLICATE("&(");
  const TymStr * str_suffix = TYM_CSTR_DUPLICATE(")");
  return tym_append_str (str_prefix, tym_append_str (csyn->name, str_suffix));
}

void
tym_csyntax_free(const struct TymCSyntax * csyn)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  free((void *)csyn);
#pragma GCC diagnostic pop
}

const struct TymCSyntax *
tym_csyntax_atom(struct TymSymGen * namegen, const struct TymAtom * atom)
{
  struct TymCSyntax * result = malloc(sizeof(*result));

  const char * str_buf_args = tym_decode_str(TymEmptyString);
  const struct TymCSyntax * sub_csyns[atom->arity];
  const TymStr * array[atom->arity];
  for (int i = 0; i < atom->arity; i++) {
    sub_csyns[i] = tym_csyntax_term(namegen, atom->args[i]);

    str_buf_args = tym_decode_str(tym_append_str_destructive2(sub_csyns[i]->serialised, tym_encode_str(str_buf_args)));

    array[i] = tym_csyntax_address_of(sub_csyns[i]);
  }

  const TymStr * args_identifier = NULL;
  const TymStr * array_type = TYM_CSTR_DUPLICATE("struct TymTerm *");
  const TymStr * array_str = tym_array_of(namegen, &args_identifier, atom->arity, array_type, array);

  str_buf_args = tym_decode_str(tym_append_str_destructive1(tym_encode_str(str_buf_args), array_str));

  const char * predicate = tym_decode_str(atom->predicate);
  char * str_buf = malloc(sizeof(*str_buf) * TYM_BUF_SIZE);

  result->name = tym_mk_new_var(namegen);
  result->type = TYM_CSTR_DUPLICATE("struct TymAtom");

  int buf_occupied = sprintf(str_buf, "%s %s = (%s){.predicate = TYM_CSTR_DUPLICATE(\"%s\"), .arity = %d, .args = %s};\n",
    tym_decode_str(result->type), tym_decode_str(result->name),
    tym_decode_str(result->type), predicate, atom->arity,
    tym_decode_str(args_identifier));
  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);

  const char * pretrimmed_str_buf = tym_decode_str(tym_append_str_destructive(tym_encode_str(str_buf_args), tym_encode_str(str_buf)));
  char * trimmed_str_buf = malloc(sizeof(*trimmed_str_buf) * (1 + strlen(pretrimmed_str_buf)));
  strcpy(trimmed_str_buf, pretrimmed_str_buf);
  result->serialised = tym_encode_str(trimmed_str_buf);
  result->kind = TYM_ATOM;
  result->original = atom;

  for (int i = 0; i < atom->arity; i++) {
    tym_csyntax_free(sub_csyns[i]);
  }

  return result;
}

const TymStr *
tym_array_of(struct TymSymGen * namegen, const TymStr ** result_name, size_t array_size, const TymStr * expression_type, const TymStr ** expression_strs)
{
  if (0 == array_size) {
    *expression_strs = NULL;
    *result_name = TYM_CSTR_DUPLICATE("NULL");
    return TymEmptyString;
  }

  const TymStr * sep = TYM_CSTR_DUPLICATE(", ");
  const char * str_buf_args = tym_decode_str(TymEmptyString);
  for (size_t i = 0; i < array_size; i++) {
    if (TymEmptyString != tym_encode_str(str_buf_args)) {
      str_buf_args = tym_decode_str(tym_append_str_destructive2(sep, tym_encode_str(str_buf_args)));
    }
    str_buf_args = tym_decode_str(tym_append_str_destructive2(expression_strs[i], tym_encode_str(str_buf_args))); // NOTE using tym_append_str_destructive would not have worked here, since we'd have also destroyed expression_strs[i]; so instead I explicitly specify wrt which parameter the append function acts destructively.
  }

  *result_name = tym_mk_new_var(namegen);
  char * str_buf = malloc(sizeof(*str_buf) * TYM_BUF_SIZE);
  int buf_occupied = sprintf(str_buf, "%s %s[%lu] = {%s};\n",
    tym_decode_str(expression_type), tym_decode_str(*result_name),
    array_size, str_buf_args);

  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);

  char * trimmed_str_buf = malloc(sizeof(*trimmed_str_buf) * (1 + strlen(str_buf)));
  strcpy(trimmed_str_buf, str_buf);
  free(str_buf);
  return tym_encode_str(trimmed_str_buf);
}

const struct TymCSyntax * tym_csyntax_clause(struct TymSymGen * namegen, const struct TymClause * cl)
{
  struct TymCSyntax * result = malloc(sizeof(*result));

  const char * str_buf_args = tym_decode_str(TymEmptyString);
  const struct TymCSyntax * sub_csyns[cl->body_size];
  const TymStr * array[cl->body_size];
  for (int i = 0; i < cl->body_size; i++) {
    sub_csyns[i] = tym_csyntax_atom(namegen, cl->body[i]);

    str_buf_args = tym_decode_str(tym_append_str_destructive2(sub_csyns[i]->serialised, tym_encode_str(str_buf_args)));

    array[i] = tym_csyntax_address_of(sub_csyns[i]);
  }

  const TymStr * args_identifier = NULL;
  const TymStr * array_type = TYM_CSTR_DUPLICATE("struct TymAtom *");
  const TymStr * array_str = tym_array_of(namegen, &args_identifier, cl->body_size, array_type, array);

  str_buf_args = tym_decode_str(tym_append_str_destructive1(tym_encode_str(str_buf_args), array_str));

  const struct TymCSyntax * head = tym_csyntax_atom(namegen, cl->head);

  str_buf_args = tym_decode_str(tym_append_str_destructive1(tym_encode_str(str_buf_args), head->serialised));

  char * str_buf = malloc(sizeof(*str_buf) * TYM_BUF_SIZE);

  result->name = tym_mk_new_var(namegen);
  result->type = TYM_CSTR_DUPLICATE("struct TymClause");

  int buf_occupied = sprintf(str_buf, "%s %s = (%s){.head = %s, .body_size = %d, .body = %s};\n",
    tym_decode_str(result->type), tym_decode_str(result->name),
    tym_decode_str(result->type), tym_decode_str(tym_csyntax_address_of(head)), cl->body_size,
    tym_decode_str(args_identifier));
  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);

  const char * pretrimmed_str_buf = tym_decode_str(tym_append_str_destructive(tym_encode_str(str_buf_args), tym_encode_str(str_buf)));
  char * trimmed_str_buf = malloc(sizeof(*trimmed_str_buf) * (1 + strlen(pretrimmed_str_buf)));
  strcpy(trimmed_str_buf, pretrimmed_str_buf);
  result->serialised = tym_encode_str(trimmed_str_buf);
  result->kind = TYM_CLAUSE;
  result->original = cl;

  tym_csyntax_free(head);
  for (int i = 0; i < cl->body_size; i++) {
    tym_csyntax_free(sub_csyns[i]);
  }

  return result;
}

const struct TymCSyntax * tym_csyntax_program(struct TymSymGen * namegen, const struct TymProgram * prog)
{
  struct TymCSyntax * result = malloc(sizeof(*result));

  const char * str_buf_args = tym_decode_str(TymEmptyString);
  const struct TymCSyntax * sub_csyns[prog->no_clauses];
  const TymStr * array[prog->no_clauses];
  for (int i = 0; i < prog->no_clauses; i++) {
    sub_csyns[i] = tym_csyntax_clause(namegen, prog->program[i]);

    str_buf_args = tym_decode_str(tym_append_str_destructive2(sub_csyns[i]->serialised, tym_encode_str(str_buf_args)));

    array[i] = tym_csyntax_address_of(sub_csyns[i]);
  }

  const TymStr * args_identifier = NULL;
  const TymStr * array_type = TYM_CSTR_DUPLICATE("struct TymClause *");
  const TymStr * array_str = tym_array_of(namegen, &args_identifier, prog->no_clauses, array_type, array);

  str_buf_args = tym_decode_str(tym_append_str_destructive1(tym_encode_str(str_buf_args), array_str));

  char * str_buf = malloc(sizeof(*str_buf) * TYM_BUF_SIZE);

  result->name = tym_mk_new_var(namegen);
  result->type = TYM_CSTR_DUPLICATE("struct TymProgram");

  int buf_occupied = sprintf(str_buf, "%s %s = (%s){.no_clauses = %d, .program = %s};\n",
    tym_decode_str(result->type), tym_decode_str(result->name),
    tym_decode_str(result->type), prog->no_clauses,
    tym_decode_str(args_identifier));
  assert(buf_occupied > 0);
  assert(strlen(str_buf) == (unsigned long)buf_occupied);

  const char * pretrimmed_str_buf = tym_decode_str(tym_append_str_destructive(tym_encode_str(str_buf_args), tym_encode_str(str_buf)));
  char * trimmed_str_buf = malloc(sizeof(*trimmed_str_buf) * (1 + strlen(pretrimmed_str_buf)));
  strcpy(trimmed_str_buf, pretrimmed_str_buf);
  result->serialised = tym_encode_str(trimmed_str_buf);
  result->kind = TYM_PROGRAM;
  result->original = prog;

  for (int i = 0; i < prog->no_clauses; i++) {
    tym_csyntax_free(sub_csyns[i]);
  }

  return result;
}
