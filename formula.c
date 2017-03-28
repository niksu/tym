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
mk_fmla_const(bool b)
{
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  assert(NULL != result);

  result->kind = FMLA_CONST;
  result->param.const_value = b;

  return result;
}

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

struct fmla_t *
mk_fmla_imply(struct fmla_t * antecedent, struct fmla_t * consequent)
{
  return mk_fmla_or(mk_fmla_not(antecedent), consequent);
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
  case FMLA_CONST:
    if (fmla->param.const_value) {
      sprintf(&(buf[l]), "true");
    } else {
      sprintf(&(buf[l]), "false");
    }
    *remaining -= strlen(&(buf[l]));
    l += strlen(&(buf[l]));
    break;
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

struct fmlas_t *
mk_fmla_cell(struct fmla_t * fmla, struct fmlas_t * next)
{
  assert(NULL != fmla);

  struct fmlas_t * fs = malloc(sizeof(struct fmlas_t));
  assert(NULL != fs);

  fs->fmla = fmla;
  fs->next = next;
  return fs;
}

struct var_gen_t *
mk_var_gen(const char * prefix)
{
  struct var_gen_t * result = malloc(sizeof(struct var_gen_t));
  result->prefix = prefix;
  result->index = 0;
  return result;
}

char *
mk_new_var(struct var_gen_t * vg)
{
  size_t i = strlen(vg->prefix);
  int max_width = 10/*FIXME const*/;
  char * result = malloc(i + 1 + max_width);
  strcpy(result, vg->prefix);
  snprintf(result + i, max_width, "%lu", vg->index);
  vg->index += 1;
  return result;
}

bool
fmla_is_atom(struct fmla_t * fmla)
{
  switch (fmla->kind) {
  case FMLA_ATOM:
    return true;
  default:
    return false;
  }
}

struct fmla_atom_t *
fmla_as_atom(struct fmla_t * fmla)
{
  if (fmla_is_atom(fmla)) {
    return fmla->param.atom;
  } else {
    return NULL;
  }
}

struct fmla_t *
mk_abstract_vars(struct fmla_t * at, struct var_gen_t * vg, struct valuation_t ** v)
{
  struct fmla_atom_t * atom = fmla_as_atom(at);
  assert(NULL != atom);

  char ** var_args = malloc(sizeof(char **) * atom->arity);
  *v = NULL;

  struct valuation_t * v_cursor;
  for (int i = 0; i < atom->arity; i++) {
    if (0 == i) {
      *v = malloc(sizeof(struct valuation_t));
      v_cursor = *v;
    } else {
      assert(NULL != v_cursor);
      v_cursor->next = malloc(sizeof(struct valuation_t));
      v_cursor = v_cursor->next;
    }
    v_cursor->val = atom->predargs[i];
    v_cursor->var = mk_new_var(vg);
    *(var_args + i) = v_cursor->var;
  }

  return mk_fmla_atom(atom->pred_name, atom->arity, var_args);
}


size_t
valuation_str(struct valuation_t * v, size_t * remaining, char * buf)
{
  size_t l = 0;
  struct valuation_t * v_cursor = v;

  while (NULL != v_cursor) {
    size_t l_sub;
    l_sub = my_strcpy(buf + l, v_cursor->var, remaining);
    // FIXME check and react to errors.
    l += l_sub;
    buf[(*remaining)--, l++] = '=';
    l_sub = my_strcpy(buf + l, v_cursor->val, remaining);
    // FIXME check and react to errors.
    l += l_sub;

    v_cursor = v_cursor->next;

    if (NULL != v_cursor) {
      buf[(*remaining)--, l++] = ',';
      buf[(*remaining)--, l++] = ' ';
    }
  }

  return l;
}
