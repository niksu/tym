/*
 * Representation of formulas.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <assert.h>

#include "ast.h"
#include "formula.h"
#include "tym.h"

struct fmla_t *
mk_fmla_atom(char * pred_name, uint8_t arity, char ** predargs)
{
  struct fmla_atom_t * result_content = malloc(sizeof(struct fmla_atom_t));
  assert(NULL != result_content);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  assert(NULL != result);

  result->kind = FMLA_ATOM;
  result->param.atom = result_content;

  result_content->pred_name = pred_name;
  result_content->arity = arity;
  result_content->predargs = predargs;

  return result;
}

struct fmla_t *
mk_fmla_quant(const char * bv, struct fmla_t * body)
{
  struct fmla_quant_t * result_content = (struct fmla_quant_t *)malloc(sizeof(struct fmla_quant_t *));
  struct fmla_t * result = (struct fmla_t *)malloc(sizeof(struct fmla_t *));
  result_content->bv = bv;
  result_content->body = body;
  result->kind = FMLA_ALL;
  result->param.quant = result_content;
  return result;
}

struct fmla_t *
mk_fmla_not(struct fmla_t * subfmla)
{
  struct fmla_t ** result_content = (struct fmla_t **)malloc(sizeof(struct fmla_t **) * 1);
  struct fmla_t * result = (struct fmla_t *)malloc(sizeof(struct fmla_t *));
  result->kind = FMLA_NOT;
  *result_content = subfmla;
  result->param.args = result_content;
  return result;
}

struct fmla_t *
mk_fmla_and(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR)
{
  struct fmla_t ** result_content = (struct fmla_t **)malloc(sizeof(struct fmla_t **) * 2);
  struct fmla_t * result = (struct fmla_t *)malloc(sizeof(struct fmla_t *));
  result->kind = FMLA_AND;
  *result_content = subfmlaL;
  *(result_content + 1) = subfmlaR;
  result->param.args = result_content;
  return result;
}

struct fmla_t *
mk_fmla_or(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR)
{
  struct fmla_t ** result_content = (struct fmla_t **)malloc(sizeof(struct fmla_t **) * 2);
  struct fmla_t * result = (struct fmla_t *)malloc(sizeof(struct fmla_t *));
  result->kind = FMLA_OR;
  *result_content = subfmlaL;
  *(result_content + 1) = subfmlaR;
  result->param.args = result_content;
  return result;
}

size_t
fmla_atom_str(struct fmla_atom_t * at, size_t * remaining, char * buf)
{
  size_t l = 0;
  char * dst = buf + l;
  char * src = at->pred_name;
  // FIXME inline expressions above into expression below?
  size_t l_sub = my_strcpy(dst, src, remaining);
  // FIXME check and react to errors.
  l += l_sub;

  for (int i = 0; i < at->arity; i++) {
    buf[(*remaining)--, l++] = ' ';
    char * dst = buf + l;
    char * src = at->predargs[i];
    // FIXME inline expressions above into expression below?
    size_t l_sub = my_strcpy(dst, src, remaining);
    // FIXME check and react to errors.
    l += l_sub;
  }
  return l;
}

size_t
fmla_quant_str(struct fmla_quant_t * quant, size_t * remaining, char * buf)
{
  size_t l = 0;
  const char * uni_type = "Universe"; // FIXME const
  buf[(*remaining)--, l++] = '(';

  buf[(*remaining)--, l++] = '(';
  // FIXME check and react to errors.
  l += my_strcpy(buf + l, quant->bv, remaining);
  buf[(*remaining)--, l++] = ' ';
  // FIXME check and react to errors.
  l += my_strcpy(buf + l, uni_type, remaining);
  buf[(*remaining)--, l++] = ')';

  buf[(*remaining)--, l++] = ')';
  buf[(*remaining)--, l++] = ' ';

  buf[(*remaining)--, l++] = '(';
  // FIXME check and react to errors.
  l += fmla_str(quant->body, remaining, buf + l);
  buf[(*remaining)--, l++] = ')';
  return l;
}

size_t
fmla_junction_str(struct fmla_t * fmlaL, struct fmla_t * fmlaR, size_t * remaining, char * buf)
{
  size_t l = 0;
  l += fmla_str(fmlaL, remaining, buf + l);
  buf[(*remaining)--, l++] = ')';
  buf[(*remaining)--, l++] = ' ';
  buf[(*remaining)--, l++] = '(';
  l += fmla_str(fmlaR, remaining, buf + l);
  buf[(*remaining)--, l++] = ')';
  return l;
}

size_t
fmla_str(struct fmla_t * fmla, size_t * remaining, char * buf)
{
  size_t l = 0;

  switch (fmla->kind) {
  case FMLA_ATOM:
    l = fmla_atom_str(fmla->param.atom, remaining, buf + l);
    break;
  case FMLA_AND:
    sprintf(&(buf[l]), "and (");
    *remaining -= strlen(&(buf[l]));
    l += strlen(&(buf[l]));
    l += fmla_junction_str(fmla->param.args[0], fmla->param.args[1], remaining, buf + l);
    break;
  case FMLA_OR:
    sprintf(&(buf[l]), "or (");
    *remaining -= strlen(&(buf[l]));
    l += strlen(&(buf[l]));
    l += fmla_junction_str(fmla->param.args[0], fmla->param.args[1], remaining, buf + l);
    break;
  case FMLA_NOT:
    sprintf(&(buf[l]), "not (");
    *remaining -= strlen(&(buf[l]));
    l += strlen(&(buf[l]));
    l += fmla_str(fmla->param.args[0], remaining, buf + l);
    buf[(*remaining)--, l++] = ')';
    break;
  case FMLA_ALL:
    sprintf(&(buf[l]), "forall ");
    *remaining -= strlen(&(buf[l]));
    l += strlen(&(buf[l]));
    l += fmla_quant_str(fmla->param.quant, remaining, buf + l);
    break;
  default:
    // FIXME fail
    break;
  }

  return l;
}

void
test_formula()
{
  char ** args = (char **)malloc(sizeof(char **) * 2);
  *args = (char *)malloc(sizeof(char *) * 10);
  *(args + 1) = (char *)malloc(sizeof(char *) * 10);
  strcpy(*args, "arg0");
  strcpy(*(args + 1), "arg1");

  for (int i = 0; i < 2; i++) {
    printf("  :%s\n", *(args + i));
  }

  struct fmla_t * test_atom = mk_fmla_atom("atom", 2, args);

  for (int i = 0; i < 2; i++) {
    printf("  ;%s\n", test_atom->param.atom->predargs[i]);
  }

  struct fmla_t * test_not = mk_fmla_not(test_atom);
  struct fmla_t * test_and = mk_fmla_and(test_not, test_atom);
  struct fmla_t * test_or = mk_fmla_or(test_not, test_and);
  struct fmla_t * test_quant = mk_fmla_quant("x", test_or);

  size_t remaining_buf_size = BUF_SIZE;
  char * buf = (char *)malloc(remaining_buf_size);
  size_t l = fmla_str(test_quant, &remaining_buf_size, buf);
  printf("test formula (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);
}
