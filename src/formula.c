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
#include "module_tests.h"
#include "util.h"

#define MAX_VAR_WIDTH 10/*FIXME make into a parameter, rather than having it be a const*/

struct buffer_write_result * Bfmla_junction_str(struct fmla_t * fmlaL, struct fmla_t * fmlaR, struct buffer_info * dst);

const struct fmla_t *
mk_fmla_const(bool b)
{
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  assert(NULL != result);

  result->kind = FMLA_CONST;
  result->param.const_value = b;

  return result;
}

const struct fmla_t *
mk_fmla_atom(const char * pred_name, uint8_t arity, struct term_t ** predargs)
{
  struct fmla_atom_t * result_content = malloc(sizeof(struct fmla_atom_t));
  assert(NULL != result_content);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  assert(NULL != result);

  // FIXME can avoid copying?
  char * pred_name_copy = malloc(sizeof(char) * (strlen(pred_name) + 1));
  assert(NULL != pred_name_copy);
  strcpy(pred_name_copy, pred_name);

  struct term_t ** predargs_copy = NULL;

  if (arity > 0) {
    predargs_copy = malloc(sizeof(struct term_t *) * arity);
  }

  for (int i = 0; i < arity; i++) {
    predargs_copy[i] = copy_term(predargs[i]);
  }

  result->kind = FMLA_ATOM;
  result->param.atom = result_content;

  result_content->pred_name = pred_name_copy;
  result_content->arity = arity;
  result_content->predargs = predargs_copy;

  return result;
}

const struct fmla_t *
mk_fmla_atom_varargs(const char * pred_name, uint8_t arity, ...)
{
  struct term_t ** args = malloc(sizeof(struct term_t *) * arity);
  va_list varargs;
  va_start(varargs, arity);
  for (int i = 0; i < arity; i++) {
    args[i] = va_arg(varargs, struct term_t *);
  }
  va_end(varargs);

  const struct fmla_t * result = mk_fmla_atom(pred_name, arity, args);
  for (int i = 0; i < arity; i++) {
    // NOTE we don't call free_term(*args[i]), since we shouldn't free
    //      memory that's owned elsewhere.
    free(args[i]);
  }
  free(args);

  return result;
}

const struct fmla_t *
mk_fmla_quant(const char * bv, const struct fmla_t * body)
{
  assert(NULL != bv);
  assert(NULL != body);
  struct fmla_quant_t * result_content = malloc(sizeof(struct fmla_quant_t));
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  char * bv_copy = malloc(sizeof(char) * (strlen(bv) + 1));
  strcpy(bv_copy, bv);
  struct fmla_t * body_copy = copy_fmla(body);
  result_content->bv = bv_copy;
  result_content->body = body_copy;
  *result = (struct fmla_t){.kind = FMLA_EX, .param.quant = result_content};
  return result;
}

const struct fmla_t *
mk_fmla_not(const struct fmla_t * subfmla)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 1);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result_content[0] = copy_fmla(subfmla);
  *result = (struct fmla_t){.kind = FMLA_NOT, .param.args = result_content};
  return result;
}

const struct fmla_t *
mk_fmla_and(const struct fmla_t * subfmlaL, const struct fmla_t * subfmlaR)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 2);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result->kind = FMLA_AND;
  result_content[0] = copy_fmla(subfmlaL);
  result_content[1] = copy_fmla(subfmlaR);
  result->param.args = result_content;
  return result;
}

const struct fmla_t *
mk_fmla_or(const struct fmla_t * subfmlaL, const struct fmla_t * subfmlaR)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 2);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result->kind = FMLA_OR;
  result_content[0] = copy_fmla(subfmlaL);
  result_content[1] = copy_fmla(subfmlaR);
  result->param.args = result_content;
  return result;
}

const struct fmla_t *
mk_fmla_ands(const struct fmlas_t * fmlas)
{
  const struct fmla_t * result;
  const struct fmlas_t * cursor = fmlas;
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

const struct fmla_t *
mk_fmla_ors(const struct fmlas_t * fmlas)
{
  const struct fmla_t * result;
  const struct fmlas_t * cursor = fmlas;
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

const struct fmla_t *
mk_fmla_imply(struct fmla_t * antecedent, struct fmla_t * consequent)
{
  const struct fmla_t * subfmla = mk_fmla_not(antecedent);
  const struct fmla_t * result = mk_fmla_or(subfmla, consequent);
  free_fmla(subfmla);
  return result;
}

struct buffer_write_result *
Bfmla_atom_str(struct fmla_atom_t * at, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = buf_strcpy(dst, at->pred_name);
  assert(is_ok_buffer_write_result(res));
  free(res);

  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

  for (int i = 0; i < at->arity; i++) {
    if (have_space(dst, 1)) {
      unsafe_buffer_char(dst, ' ');
    } else {
      return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
    }

    res = Bterm_to_str(at->predargs[i], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    unsafe_dec_idx(dst, 1); // chomp the trailing \0.
  }

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
}

struct buffer_write_result *
Bfmla_quant_str(struct fmla_quant_t * quant, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  if (have_space(dst, 2)) {
    unsafe_buffer_str(dst, "((");
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }

  struct buffer_write_result * res = buf_strcpy(dst, quant->bv);
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  res = buf_strcpy(dst, universe_ty);
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, ')'); // replace the trailing \0.

  unsafe_buffer_str(dst, ") ");
  unsafe_dec_idx(dst, 1); // chomp the trailing \0.

  res = Bfmla_str(quant->body, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct buffer_write_result *
Bfmla_junction_str(struct fmla_t * fmlaL, struct fmla_t * fmlaR, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = Bfmla_str(fmlaL, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  res = Bfmla_str(fmlaR, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct buffer_write_result *
Bfmla_str(const struct fmla_t * fmla, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  const size_t fmla_sz = fmla_size(fmla);
  if (fmla_sz > 1) {
    if (have_space(dst, 1)) {
      unsafe_buffer_char(dst, '(');
    } else {
      return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
    }
  }

  struct buffer_write_result * res = NULL;

  switch (fmla->kind) {
  case FMLA_CONST:
    if (fmla->param.const_value) {
      res = buf_strcpy(dst, "true");
    } else {
      res = buf_strcpy(dst, "false");
    }
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_ATOM:
    res = Bfmla_atom_str(fmla->param.atom, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_AND:
    res = buf_strcpy(dst, "and");
    assert(is_ok_buffer_write_result(res));
    free(res);
    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = Bfmla_junction_str(fmla->param.args[0], fmla->param.args[1], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_OR:
    res = buf_strcpy(dst, "or");
    assert(is_ok_buffer_write_result(res));
    free(res);
    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = Bfmla_junction_str(fmla->param.args[0], fmla->param.args[1], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_NOT:
    res = buf_strcpy(dst, "not");
    assert(is_ok_buffer_write_result(res));
    free(res);
    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = Bfmla_str(fmla->param.args[0], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_EX:
    res = buf_strcpy(dst, "exists");
    assert(is_ok_buffer_write_result(res));
    free(res);
    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = Bfmla_quant_str(fmla->param.quant, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  default:
    return mkerrval_buffer_write_result(NON_BUFF_ERROR);
  }

  if (fmla_sz > 1) {
    safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
  } else {
    unsafe_dec_idx(dst, 1);
  }

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
}

DEFINE_LIST_MK(fmla, fmla, struct fmla_t, struct fmlas_t, /*no const*/)

struct sym_gen_t *
mk_sym_gen(const char * prefix)
{
  struct sym_gen_t * result = malloc(sizeof(struct sym_gen_t));
  char * prefix_copy = malloc(sizeof(char) * (strlen(prefix) + 1));
  strcpy(prefix_copy, prefix);
  // FIXME use naming convention to indicate which parameters are copied, and which aren't.
  result->prefix = prefix_copy;
  result->index = 0;
  return result;
}

struct sym_gen_t *
copy_sym_gen(const struct sym_gen_t * const orig)
{
  struct sym_gen_t * result = malloc(sizeof(struct sym_gen_t));
  char * prefix_copy = malloc(sizeof(char) * (strlen(orig->prefix) + 1));
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
fmla_is_atom(const struct fmla_t * fmla)
{
  switch (fmla->kind) {
  case FMLA_ATOM:
    return true;
  default:
    return false;
  }
}

struct fmla_atom_t *
fmla_as_atom(const struct fmla_t * fmla)
{
  if (fmla_is_atom(fmla)) {
    return fmla->param.atom;
  } else {
    return NULL;
  }
}

const struct fmla_t *
mk_abstract_vars(const struct fmla_t * at, struct sym_gen_t * vg, struct valuation_t ** v)
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

  free(var_args);
  const struct fmla_t * result = mk_fmla_atom(atom->pred_name, atom->arity, var_args_T);
  for (int i = 0; i < atom->arity; i++) {
    free_term(*var_args_T[i]);
    free(var_args_T[i]);
  }
  free(var_args_T); // since mk_fmla_atom copies the formula.
  return result;
}

struct buffer_write_result *
Bvaluation_str(struct valuation_t * v, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct valuation_t * v_cursor = v;

  struct buffer_write_result * res = NULL;

  while (NULL != v_cursor) {
    res = buf_strcpy(dst, v_cursor->var);
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, '='); // replace the trailing \0.

    res = Bterm_to_str(v_cursor->val, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);

    v_cursor = v_cursor->next;

    if (NULL != v_cursor) {
      safe_buffer_replace_last(dst, ','); // replace the trailing \0.

      if (have_space(dst, 1)) {
        unsafe_buffer_char(dst, ' ');
      } else {
        return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
      }
    }
  }

  if (have_space(dst, 1)) {
    unsafe_buffer_char(dst, '\0');
    return mkval_buffer_write_result(dst->idx - initial_idx);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
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
  assert (NULL != q->bv);
  free((void *)q->bv);

  assert (NULL != q->body);
  free_fmla(q->body);

  free(q);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_fmla(const struct fmla_t * fmla)
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
    assert(false);
    break;
  }

  free((void *)fmla);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_fmlas(const struct fmlas_t * fmlas)
{
  assert(NULL != fmlas->fmla);
  free_fmla(fmlas->fmla);
  free_fmlas(fmlas->next);

  free((void *)fmlas);
}
#pragma GCC diagnostic pop

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

const struct fmlas_t *
mk_fmlas(uint8_t no_fmlas, ...)
{
  va_list varargs;
  va_start(varargs, no_fmlas);
  const struct fmlas_t * result = NULL;
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_const(fmla->param.const_value);
#pragma GCC diagnostic pop
    break;
  case FMLA_ATOM:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_atom(fmla->param.atom->pred_name, fmla->param.atom->arity, fmla->param.atom->predargs);
#pragma GCC diagnostic pop
    break;
  case FMLA_AND:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_and(fmla->param.args[0], fmla->param.args[1]);
#pragma GCC diagnostic pop
    break;
  case FMLA_OR:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_or(fmla->param.args[0], fmla->param.args[1]);
#pragma GCC diagnostic pop
    break;
  case FMLA_NOT:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_not(fmla->param.args[0]);
#pragma GCC diagnostic pop
    break;
  case FMLA_EX:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_quant(fmla->param.quant->bv, fmla->param.quant->body);
#pragma GCC diagnostic pop
    break;
  default:
    assert(false);
    break;
  }

  return result;
}

void
test_formula(void)
{
  printf("***test_formula***\n");
  struct term_t ** args = malloc(sizeof(struct term_t *) * 2);
  *args = mk_term(CONST, "arg0");
  *(args + 1) = mk_term(CONST, "arg1");

  for (int i = 0; i < 2; i++) {
    printf("  :%s\n", args[i]->identifier);
  }

  const struct fmla_t * test_atom = mk_fmla_atom("atom", 2, args);

  for (int i = 0; i < 2; i++) {
    printf("  ;%s\n", test_atom->param.atom->predargs[i]->identifier);
  }

  const struct fmla_t * test_not = mk_fmla_not(test_atom);
  const struct fmla_t * test_and = mk_fmla_and(test_not, test_atom);
  const struct fmla_t * test_or = mk_fmla_or(test_not, test_and);
  const struct fmla_t * test_quant = mk_fmla_quant("x", test_or);

  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = Bfmla_str(test_quant, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test formula (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  free_buffer(outbuf);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);

  free(args[0]);
  free(args[1]);
  free(args);
  free_fmla(test_quant);
  free_fmla(test_and);
  free_fmla(test_atom);
  free_fmla(test_not);

  test_atom = mk_fmla_atom_varargs("testpred", 4, mk_const("ta1"), mk_const("ta2"), mk_const("ta3"), mk_const("ta4"));
  const struct fmlas_t * test_fmlas = mk_fmlas(3, test_atom, test_atom, test_atom);
  const struct fmla_t * test_and2 = mk_fmla_ands(test_fmlas);
  const struct fmla_t * test_or2 = mk_fmla_ors(test_fmlas);

  outbuf = mk_buffer(BUF_SIZE);
  res = Bfmla_str(test_atom, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test_atom formula (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  free_buffer(outbuf);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);

  outbuf = mk_buffer(BUF_SIZE);
  res = Bfmla_str(test_and2, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test_and2 formula (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  free_buffer(outbuf);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);

  outbuf = mk_buffer(BUF_SIZE);
  res = Bfmla_str(test_or, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test_or formula (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  free_buffer(outbuf);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);

  free_fmla(test_atom);
  free_fmla(test_and2);
  free_fmla(test_or);
  free_fmla(test_or2);
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

const struct fmla_t *
mk_fmla_quants(const struct terms_t * const vars, const struct fmla_t * body)
{
  const struct fmla_t * result = copy_fmla(body);
  const struct terms_t * cursor = vars;
  while (NULL != cursor) {
    assert(VAR == cursor->term->kind);
    const struct fmla_t * pre_result = mk_fmla_quant(cursor->term->identifier, result);

    free_fmla(result);

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
    assert(false);
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
    assert(false);
    break;
  }
  return result;
}
