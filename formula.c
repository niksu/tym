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

#define MAX_VAR_WIDTH 10/*FIXME const*/

void test_formula(void);
size_t fmla_junction_str(struct fmla_t * fmlaL, struct fmla_t * fmlaR, size_t * remaining, char * buf);

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
mk_fmla_atom(const char * pred_name, uint8_t arity, struct term_t ** predargs)
{
  struct fmla_atom_t * result_content = malloc(sizeof(struct fmla_atom_t));
  assert(NULL != result_content);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  assert(NULL != result);

  // FIXME can avoid copying?
  char * pred_name_copy = malloc(sizeof(char) * strlen(pred_name));
  assert(NULL != pred_name_copy);
  strcpy(pred_name_copy, pred_name);
  struct term_t ** predargs_copy = malloc(sizeof(struct term_t *) * arity);
  for (int i = 0; i < arity; i++) {
    predargs_copy[i] = copy_term(predargs[i]);
  }

  if (0 == arity) {
    predargs_copy = NULL;
  }

  result->kind = FMLA_ATOM;
  result->param.atom = result_content;

  result_content->pred_name = pred_name_copy;
  result_content->arity = arity;
  result_content->predargs = predargs_copy;

  return result;
}

struct fmla_t *
mk_fmla_atom_varargs(const char * pred_name, uint8_t arity, ...)
{
  struct term_t ** args = malloc(sizeof(struct term_t *) * arity);
  va_list varargs;
  va_start(varargs, arity);
  for (int i = 0; i < arity; i++) {
    args[i] = va_arg(varargs, struct term_t *);
  }
  va_end(varargs);
  return mk_fmla_atom(pred_name, arity, args);
}

struct fmla_t *
mk_fmla_quant(const char * bv, struct fmla_t * body)
{
  assert(NULL != bv);
  assert(NULL != body);
  struct fmla_quant_t * result_content = malloc(sizeof(struct fmla_quant_t));
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  char * bv_copy = malloc(sizeof(char) * strlen(bv));
  strcpy(bv_copy, bv);
  struct fmla_t * body_copy = copy_fmla(body);
  result_content->bv = bv_copy;
  result_content->body = body_copy;
  result->kind = FMLA_EX;
  result->param.quant = result_content;
  return result;
}

struct fmla_t *
mk_fmla_not(struct fmla_t * subfmla)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 1);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result->kind = FMLA_NOT;
  result_content[0] = copy_fmla(subfmla);
  result->param.args = result_content;
  return result;
}

struct fmla_t *
mk_fmla_and(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 2);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result->kind = FMLA_AND;
  result_content[0] = copy_fmla(subfmlaL);
  result_content[1] = copy_fmla(subfmlaR);
  result->param.args = result_content;
  return result;
}

struct fmla_t *
mk_fmla_or(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 2);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result->kind = FMLA_OR;
  result_content[0] = copy_fmla(subfmlaL);
  result_content[1] = copy_fmla(subfmlaR);
  result->param.args = result_content;
  return result;
}

struct fmla_t *
mk_fmla_ands(struct fmlas_t * fmlas)
{
  struct fmla_t * result;
  struct fmlas_t * cursor = fmlas;
  if (NULL == cursor) {
    result = mk_fmla_const(true);
  } else {
    if (NULL == fmlas->next) {
      result = fmlas->fmla;
    } else {
      result = mk_fmla_and(fmlas->fmla, fmlas->next->fmla);
      cursor = fmlas->next->next;
      while (NULL != cursor) {
        result = mk_fmla_and(result, cursor->fmla);
        cursor = cursor->next;
      }
    }
  }
  return result;
}

struct fmla_t *
mk_fmla_ors(struct fmlas_t * fmlas)
{
  struct fmla_t * result;
  struct fmlas_t * cursor = fmlas;
  if (NULL == cursor) {
    result = mk_fmla_const(false);
  } else {
    if (NULL == fmlas->next) {
      result = fmlas->fmla;
    } else {
      result = mk_fmla_or(fmlas->fmla, fmlas->next->fmla);
      cursor = fmlas->next->next;
      while (NULL != cursor) {
        result = mk_fmla_or(result, cursor->fmla);
        cursor = cursor->next;
      }
    }
  }
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
    l_sub = term_to_str(at->predargs[i], remaining, buf + l);
    // FIXME check and react to errors.
    l += l_sub;
  }

  buf[l] = '\0';
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

  // FIXME check and react to errors.
  l += fmla_str(quant->body, remaining, buf + l);

  buf[l] = '\0';
  return l;
}

size_t
fmla_junction_str(struct fmla_t * fmlaL, struct fmla_t * fmlaR, size_t * remaining, char * buf)
{
  size_t l = 0;
  l += fmla_str(fmlaL, remaining, buf + l);
  buf[(*remaining)--, l++] = ' ';
  l += fmla_str(fmlaR, remaining, buf + l);

  buf[l] = '\0';
  return l;
}

size_t
fmla_str(struct fmla_t * fmla, size_t * remaining, char * buf)
{
  size_t l = 0;

  const size_t fmla_sz = fmla_size(fmla);
  if (fmla_sz > 1) {
    buf[(*remaining)--, l++] = '(';
  }

  switch (fmla->kind) {
  case FMLA_CONST:
    if (fmla->param.const_value) {
      sprintf(buf + l, "true");
    } else {
      sprintf(buf + l, "false");
    }
    *remaining -= strlen(buf + l);
    l += strlen(buf + l);
    break;
  case FMLA_ATOM:
    l += fmla_atom_str(fmla->param.atom, remaining, buf + l);
    break;
  case FMLA_AND:
    sprintf(buf + l, "and ");
    *remaining -= strlen(buf + l);
    l += strlen(buf + l);
    l += fmla_junction_str(fmla->param.args[0], fmla->param.args[1], remaining, buf + l);
    break;
  case FMLA_OR:
    sprintf(buf + l, "or ");
    *remaining -= strlen(buf + l);
    l += strlen(buf + l);
    l += fmla_junction_str(fmla->param.args[0], fmla->param.args[1], remaining, buf + l);
    break;
  case FMLA_NOT:
    sprintf(buf + l, "not ");
    *remaining -= strlen(buf + l);
    l += strlen(buf + l);
    l += fmla_str(fmla->param.args[0], remaining, buf + l);
    break;
  case FMLA_EX:
    sprintf(buf + l, "exists ");
    *remaining -= strlen(buf + l);
    l += strlen(buf + l);
    l += fmla_quant_str(fmla->param.quant, remaining, buf + l);
    break;
  default:
    // FIXME fail
    break;
  }

  if (fmla_sz > 1) {
    buf[(*remaining)--, l++] = ')';
  }

  buf[l] = '\0';
  return l;
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

struct sym_gen_t *
mk_sym_gen(const char * prefix)
{
  struct sym_gen_t * result = malloc(sizeof(struct sym_gen_t));
  result->prefix = prefix;
  result->index = 0;
  return result;
}

struct sym_gen_t *
copy_sym_gen(const struct sym_gen_t * const orig)
{
  struct sym_gen_t * result = malloc(sizeof(struct sym_gen_t));
  char * prefix_copy = malloc(sizeof(char) * strlen(orig->prefix));
  strcpy(prefix_copy, orig->prefix);
  result->prefix = prefix_copy;
  result->index = orig->index;
  return result;
}

char *
mk_new_var(struct sym_gen_t * vg)
{
  size_t i = strlen(vg->prefix);
  char * result = malloc(i + 1 + MAX_VAR_WIDTH);
  strcpy(result, vg->prefix);
  snprintf(result + i, MAX_VAR_WIDTH, "%lu", vg->index);
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
mk_abstract_vars(struct fmla_t * at, struct sym_gen_t * vg, struct valuation_t ** v)
{
  struct fmla_atom_t * atom = fmla_as_atom(at);
  assert(NULL != atom);

  char ** var_args = malloc(sizeof(char *) * atom->arity);
  struct term_t ** var_args_T = malloc(sizeof(struct term_t *) * atom->arity);
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

    v_cursor->val = copy_term(atom->predargs[i]);

    v_cursor->var = mk_new_var(vg);
    var_args[i] = v_cursor->var;
    var_args_T[i] = mk_term(VAR, v_cursor->var);

    v_cursor->next = NULL;
  }

  return mk_fmla_atom(atom->pred_name, atom->arity, var_args_T);
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

    l_sub = term_to_str(v_cursor->val, remaining, buf + l);
    // FIXME check and react to errors.
    l += l_sub;

    v_cursor = v_cursor->next;

    if (NULL != v_cursor) {
      buf[(*remaining)--, l++] = ',';
      buf[(*remaining)--, l++] = ' ';
    }
  }

  buf[l] = '\0';
  return l;
}

void
free_fmla_atom(struct fmla_atom_t * at)
{
  free(at->pred_name);

  for (int i = 0; i < at->arity; i++) {
    free(at->predargs[i]);
  }

  if (NULL != at->predargs) {
    assert(at->arity > 0);
    free(at->predargs);
  } else {
    assert(0 == at->arity);
  }

  free(at);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_fmla_quant(struct fmla_quant_t * q)
{
  // FIXME these checks are overly prudent -- turn them into asserts.
  if (NULL != q->bv) {
    free((void *)q->bv);
  }
  if (NULL != q->body) {
    free_fmla(q->body);
  }

  free(q);
}
#pragma GCC diagnostic pop

void
free_fmla(struct fmla_t * fmla)
{
  switch (fmla->kind) {
  case FMLA_CONST:
    // Nothing to free.
    break;
  case FMLA_ATOM:
    free_fmla_atom(fmla->param.atom);
    break;
  case FMLA_AND:
    free_fmla(fmla->param.args[0]);
    free_fmla(fmla->param.args[1]);
    free(fmla->param.args);
    break;
  case FMLA_OR:
    free_fmla(fmla->param.args[0]);
    free_fmla(fmla->param.args[1]);
    free(fmla->param.args);
    break;
  case FMLA_NOT:
    free_fmla(fmla->param.args[0]);
    free(fmla->param.args);
    break;
  case FMLA_EX:
    free_fmla_quant(fmla->param.quant);
    break;
  default:
    // FIXME fail
    break;
  }

  free(fmla);
}

void
free_fmlas(struct fmlas_t * fmlas)
{
  assert(NULL != fmlas->fmla);
  free_fmla(fmlas->fmla);
  free_fmlas(fmlas->next);

  free(fmlas);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_sym_gen(struct sym_gen_t * vg)
{
  assert(NULL != vg->prefix);
  free((void *)vg->prefix);

  free(vg);
}
#pragma GCC diagnostic pop

void
free_valuation(struct valuation_t * v)
{
  assert(NULL != v);

  assert(NULL != v->var);
  free(v->var);

  assert(NULL != v->val);
  free(v->val);

  if (NULL != v->next) {
    free_valuation(v->next);
  }

  free(v);
}

struct fmlas_t *
mk_fmlas(uint8_t no_fmlas, ...)
{
  va_list varargs;
  va_start(varargs, no_fmlas);
  struct fmlas_t * result = NULL;
  for (int i = 0; i < no_fmlas; i++) {
    struct fmla_t * cur_fmla = va_arg(varargs, struct fmla_t *);
    assert(NULL != cur_fmla);
    result = mk_fmla_cell(cur_fmla, result);
  }
  va_end(varargs);
  return result;
}

struct fmla_t *
copy_fmla(const struct fmla_t * const fmla)
{
  struct fmla_t * result = NULL;
  switch (fmla->kind) {
  case FMLA_CONST:
    result = mk_fmla_const(fmla->param.const_value);
    break;
  case FMLA_ATOM:
    result = mk_fmla_atom(fmla->param.atom->pred_name, fmla->param.atom->arity, fmla->param.atom->predargs);
    break;
  case FMLA_AND:
    result = mk_fmla_and(fmla->param.args[0], fmla->param.args[1]);
    break;
  case FMLA_OR:
    result = mk_fmla_or(fmla->param.args[0], fmla->param.args[1]);
    break;
  case FMLA_NOT:
    result = mk_fmla_not(fmla->param.args[0]);
    break;
  case FMLA_EX:
    result = mk_fmla_quant(fmla->param.quant->bv, fmla->param.quant->body);
    break;
  default:
    // FIXME fail
    break;
  }

  return result;
}

void
test_formula(void)
{
  struct term_t ** args = malloc(sizeof(struct term_t *) * 2);
  *args = mk_term(CONST, "arg0");
  *(args + 1) = mk_term(CONST, "arg1");

  for (int i = 0; i < 2; i++) {
    printf("  :%s\n", args[i]->identifier);
  }

  struct fmla_t * test_atom = mk_fmla_atom("atom", 2, args);

  for (int i = 0; i < 2; i++) {
    printf("  ;%s\n", test_atom->param.atom->predargs[i]->identifier);
  }

  struct fmla_t * test_not = mk_fmla_not(test_atom);
  struct fmla_t * test_and = mk_fmla_and(test_not, test_atom);
  struct fmla_t * test_or = mk_fmla_or(test_not, test_and);
  struct fmla_t * test_quant = mk_fmla_quant("x", test_or);

  size_t remaining_buf_size = BUF_SIZE;
  char * buf = malloc(remaining_buf_size);
  size_t l = fmla_str(test_quant, &remaining_buf_size, buf);
  printf("test formula (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  free(args[0]);
  free(args[1]);
  free(args);
  free_fmla(test_quant);
  free_fmla(test_and);
  free_fmla(test_atom);
  free_fmla(test_not);

  test_atom = mk_fmla_atom_varargs("testpred", 4, "ta1", "ta2", "ta3", "ta4");
  struct fmlas_t * test_fmlas = mk_fmlas(3, test_atom, test_atom, test_atom);
  test_and = mk_fmla_ands(test_fmlas);
  test_or = mk_fmla_ors(test_fmlas);

  remaining_buf_size = BUF_SIZE;
  l = fmla_str(test_atom, &remaining_buf_size, buf);
  printf("test_atom formula (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  remaining_buf_size = BUF_SIZE;
  l = fmla_str(test_and, &remaining_buf_size, buf);
  printf("test_and formula (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  remaining_buf_size = BUF_SIZE;
  l = fmla_str(test_or, &remaining_buf_size, buf);
  printf("test_or formula (size=%zu, remaining=%zu)\n|%s|\n", l, remaining_buf_size, buf);

  free_fmla(test_atom);
  free_fmla(test_and);
  free_fmla(test_or);
  free(buf);
}

struct terms_t *
filter_var_values(struct valuation_t * const v)
{
  struct terms_t * result = NULL;
  struct valuation_t * cursor = v;
  while (NULL != cursor) {
    if (VAR == cursor->val->kind) {
      struct term_t * t_copy = copy_term(cursor->val);
      result = mk_term_cell(t_copy, result);
    }
    cursor = cursor->next;
  }
  return result;
}

struct fmla_t *
mk_fmla_quants(const struct terms_t * const vars, struct fmla_t * body)
{
  struct fmla_t * result = copy_fmla(body);
  const struct terms_t * cursor = vars;
  while (NULL != cursor) {
    assert(VAR == cursor->term->kind);
    struct fmla_t * pre_result = mk_fmla_quant(cursor->term->identifier, result);
    result = pre_result;

    cursor = cursor->next;
  }

  return result;
}

size_t
valuation_len(const struct valuation_t * v)
{
  size_t l = 0;
  while (NULL != v) {
    l++;
    v = v->next;
  }
  return l;
}

struct terms_t *
arguments_of_atom(struct fmla_atom_t * fmla)
{
  struct terms_t * result = NULL;
  for (int i = fmla->arity - 1; i >= 0; i--) {
    result = mk_term_cell(copy_term(fmla->predargs[i]), result);
  }
  return result;
}

size_t
fmla_size(const struct fmla_t * const fmla)
{
  size_t result = 0;
  switch (fmla->kind) {
  case FMLA_CONST:
    result = 1;
    break;
  case FMLA_ATOM:
    result = 1 + fmla->param.atom->arity;
    break;
  case FMLA_AND:
    result = 1 + fmla_size(fmla->param.args[0]) + fmla_size(fmla->param.args[1]);
    break;
  case FMLA_OR:
    result = 1 + fmla_size(fmla->param.args[0]) + fmla_size(fmla->param.args[1]);
    break;
  case FMLA_NOT:
    result = 1 + fmla_size(fmla->param.args[0]);
    break;
  case FMLA_EX:
    result = 1 + fmla_size(fmla->param.quant->body);
    break;
  default:
    // FIXME fail
    break;
  }

  return result;
}

struct terms_t *
consts_in_fmla(const struct fmla_t * fmla, struct terms_t * acc)
{
  struct terms_t * result = acc;
  switch (fmla->kind) {
  case FMLA_CONST:
    result = acc;
    break;
  case FMLA_ATOM:
    result = mk_term_cell(mk_term(CONST, fmla->param.atom->pred_name), acc);
    for (int i = 0; i < fmla->param.atom->arity; i++) {
      struct term_t * t = fmla->param.atom->predargs[i];
      if (CONST == t->kind) {
        result = mk_term_cell(t, result);
      }
    }
    break;
  case FMLA_AND:
  case FMLA_OR:
    acc = consts_in_fmla(fmla->param.args[0], acc);
    result = consts_in_fmla(fmla->param.args[1], acc);
    break;
  case FMLA_NOT:
    result = consts_in_fmla(fmla->param.args[0], acc);
    break;
  case FMLA_EX:
    result = consts_in_fmla(fmla->param.quant->body, acc);
    break;
  default:
    // FIXME fail
    break;
  }
  return result;
}
