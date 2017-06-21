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

struct buffer_write_result * fmla_junction_str(struct fmla_t * fmlaL, struct fmla_t * fmlaR, struct buffer_info * dst);

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
mk_fmla_atom(char * pred_name, uint8_t arity, struct term_t ** predargs)
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
mk_fmla_atom_varargs(char * pred_name, uint8_t arity, ...)
{
  struct term_t ** args = NULL;

  va_list varargs;
  va_start(varargs, arity);
  if (arity > 0) {
    args = malloc(sizeof(struct term_t *) * arity);
    for (int i = 0; i < arity; i++) {
      args[i] = va_arg(varargs, struct term_t *);
    }
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
  result_content->bv = bv;
  result_content->body = body;
  *result = (struct fmla_t){.kind = FMLA_EX, .param.quant = result_content};
  return result;
}

struct fmla_t *
mk_fmla_not(struct fmla_t * subfmla)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 1);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result_content[0] = subfmla;
  *result = (struct fmla_t){.kind = FMLA_NOT, .param.args = result_content};
  return result;
}

struct fmla_t *
mk_fmla_and(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 2);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result->kind = FMLA_AND;
  result_content[0] = subfmlaL;
  result_content[1] = subfmlaR;
  result->param.args = result_content;
  return result;
}

struct fmla_t *
mk_fmla_or(struct fmla_t * subfmlaL, struct fmla_t * subfmlaR)
{
  struct fmla_t ** result_content = malloc(sizeof(struct fmla_t *) * 2);
  struct fmla_t * result = malloc(sizeof(struct fmla_t));
  result->kind = FMLA_OR;
  result_content[0] = subfmlaL;
  result_content[1] = subfmlaR;
  result->param.args = result_content;
  return result;
}

struct fmla_t *
mk_fmla_ands(struct fmlas_t * fmlas)
{
  struct fmla_t * result;
  if (NULL == fmlas) {
    result = mk_fmla_const(true);
  } else {
    if (NULL == fmlas->next) {
      result = fmlas->fmla;
      free(fmlas);
    } else {
      const struct fmlas_t * cursor = fmlas;
      const struct fmlas_t * pre_cursor = cursor;
      result = mk_fmla_and(fmlas->fmla, fmlas->next->fmla);
      cursor = fmlas->next->next;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
      free((void *)pre_cursor->next);
      free((void *)pre_cursor);
#pragma GCC diagnostic pop
      while (NULL != cursor) {
        result = mk_fmla_and(result, cursor->fmla);
        pre_cursor = cursor;
        cursor = cursor->next;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        free((void *)pre_cursor);
#pragma GCC diagnostic pop
      }
    }
  }
  return result;
}

struct fmla_t *
mk_fmla_ors(struct fmlas_t * fmlas)
{
  struct fmla_t * result;
  if (NULL == fmlas) {
    result = mk_fmla_const(false);
  } else {
    if (NULL == fmlas->next) {
      result = fmlas->fmla;
      free(fmlas);
    } else {
      const struct fmlas_t * cursor = fmlas;
      const struct fmlas_t * pre_cursor = cursor;
      result = mk_fmla_and(fmlas->fmla, fmlas->next->fmla);
      cursor = fmlas->next->next;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
      free((void *)pre_cursor->next);
      free((void *)pre_cursor);
#pragma GCC diagnostic pop
      while (NULL != cursor) {
        result = mk_fmla_and(result, cursor->fmla);
        pre_cursor = cursor;
        cursor = cursor->next;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        free((void *)pre_cursor);
#pragma GCC diagnostic pop
      }
    }
  }
  return result;
}

struct fmla_t *
mk_fmla_imply(struct fmla_t * antecedent, struct fmla_t * consequent)
{
  struct fmla_t * subfmla = mk_fmla_not(antecedent);
  struct fmla_t * result = mk_fmla_or(subfmla, consequent);
  return result;
}

struct buffer_write_result *
fmla_atom_str(struct fmla_atom_t * at, struct buffer_info * dst)
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

    res = term_to_str(at->predargs[i], dst);
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
fmla_quant_str(struct fmla_quant_t * quant, struct buffer_info * dst)
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

  res = fmla_str(quant->body, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct buffer_write_result *
fmla_junction_str(struct fmla_t * fmlaL, struct fmla_t * fmlaR, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct buffer_write_result * res = fmla_str(fmlaL, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  res = fmla_str(fmlaR, dst);
  assert(is_ok_buffer_write_result(res));
  free(res);

  return mkval_buffer_write_result(dst->idx - initial_idx);
}

struct buffer_write_result *
fmla_str(const struct fmla_t * fmla, struct buffer_info * dst)
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
    res = fmla_atom_str(fmla->param.atom, dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_AND:
    res = buf_strcpy(dst, "and");
    assert(is_ok_buffer_write_result(res));
    free(res);
    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = fmla_junction_str(fmla->param.args[0], fmla->param.args[1], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_OR:
    res = buf_strcpy(dst, "or");
    assert(is_ok_buffer_write_result(res));
    free(res);
    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = fmla_junction_str(fmla->param.args[0], fmla->param.args[1], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_NOT:
    res = buf_strcpy(dst, "not");
    assert(is_ok_buffer_write_result(res));
    free(res);
    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = fmla_str(fmla->param.args[0], dst);
    assert(is_ok_buffer_write_result(res));
    free(res);
    break;
  case FMLA_EX:
    res = buf_strcpy(dst, "exists");
    error_check_buffer_write_result(res, buff_error_msg, dst);
    free(res);
    safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = fmla_quant_str(fmla->param.quant, dst);
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

DEFINE_MUTABLE_LIST_MK(fmla, fmla, struct fmla_t, struct fmlas_t)

struct sym_gen_t *
mk_sym_gen(const char * prefix)
{
  struct sym_gen_t * result = malloc(sizeof(struct sym_gen_t));
  result->prefix = prefix;
  result->index = 0;
  return result;
}

struct sym_gen_t *
copy_sym_gen(const struct sym_gen_t * const cp_orig)
{
  struct sym_gen_t * result = malloc(sizeof(struct sym_gen_t));
  char * prefix_copy = malloc(sizeof(char) * (strlen(cp_orig->prefix) + 1));
  strcpy(prefix_copy, cp_orig->prefix);
  result->prefix = prefix_copy;
  result->index = cp_orig->index;
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

  struct term_t ** var_args_T = NULL;

  if (atom->arity > 0) {
    var_args_T = malloc(sizeof(struct term_t *) * atom->arity);
    char ** var_args = malloc(sizeof(char *) * atom->arity);
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
      var_args_T[i] = mk_term(VAR, strdup(v_cursor->var));

      v_cursor->next = NULL;
    }

    free(var_args);
  }

  return mk_fmla_atom(strdup(atom->pred_name), atom->arity, var_args_T);
}

struct buffer_write_result *
valuation_str(struct valuation_t * v, struct buffer_info * dst)
{
  size_t initial_idx = dst->idx;

  struct valuation_t * v_cursor = v;

  struct buffer_write_result * res = NULL;

  while (NULL != v_cursor) {
    res = buf_strcpy(dst, v_cursor->var);
    assert(is_ok_buffer_write_result(res));
    free(res);

    safe_buffer_replace_last(dst, '='); // replace the trailing \0.

    res = term_to_str(v_cursor->val, dst);
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
    free_term(at->predargs[i]);
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
  if (NULL != fmlas->next) {
    free_fmlas(fmlas->next);
  }

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

  char * pred_name_copy = NULL;
  struct term_t ** predargs_copy = NULL;

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
    pred_name_copy = strdup(fmla->param.atom->pred_name);
    assert(NULL != pred_name_copy);

    predargs_copy = NULL;

    if (fmla->param.atom->arity > 0) {
      predargs_copy = malloc(sizeof(struct term_t *) * fmla->param.atom->arity);
      for (int i = 0; i < fmla->param.atom->arity; i++) {
        predargs_copy[i] = copy_term(fmla->param.atom->predargs[i]);
      }
    }

    result = (struct fmla_t *)mk_fmla_atom(pred_name_copy,
        fmla->param.atom->arity, predargs_copy);
#pragma GCC diagnostic pop
    break;
  case FMLA_AND:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_and(copy_fmla(fmla->param.args[0]),
        copy_fmla(fmla->param.args[1]));
#pragma GCC diagnostic pop
    break;
  case FMLA_OR:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_or(copy_fmla(fmla->param.args[0]),
        copy_fmla(fmla->param.args[1]));
#pragma GCC diagnostic pop
    break;
  case FMLA_NOT:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_not(copy_fmla(fmla->param.args[0]));
#pragma GCC diagnostic pop
    break;
  case FMLA_EX:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct fmla_t *)mk_fmla_quant(strdup(fmla->param.quant->bv),
        copy_fmla(fmla->param.quant->body));
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
  args[0] = mk_term(CONST, strdup("arg0"));
  args[1] = mk_term(CONST, strdup("arg1"));

  for (int i = 0; i < 2; i++) {
    printf("  :%s\n", args[i]->identifier);
  }

  struct fmla_t * test_atom = mk_fmla_atom(strdup("atom"), 2, args);

  for (int i = 0; i < 2; i++) {
    printf("  ;%s\n", test_atom->param.atom->predargs[i]->identifier);
  }

  struct fmla_t * test_not = mk_fmla_not(copy_fmla(test_atom));
  struct fmla_t * test_and = mk_fmla_and(copy_fmla(test_not), copy_fmla(test_atom));
  struct fmla_t * test_or = mk_fmla_or(copy_fmla(test_not), copy_fmla(test_and));
  struct fmla_t * test_quant = mk_fmla_quant(strdup("x"), copy_fmla(test_or));

  struct buffer_info * outbuf = mk_buffer(BUF_SIZE);
  struct buffer_write_result * res = fmla_str(test_quant, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test formula (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);

  free_buffer(outbuf);
  free_fmla(test_and);
  free_fmla(test_atom);
  free_fmla(test_not);
  free_fmla(test_quant);

  struct term_t * c1 = mk_const("ta1");
  struct term_t * c2 = mk_const("ta2");
  struct term_t * c3 = mk_const("ta3");
  struct term_t * c4 = mk_const("ta4");
  test_atom = mk_fmla_atom_varargs(strdup("testpred1"), 4, c1, c2, c3, c4);
  const struct fmla_t * test_atom2 = mk_fmla_atom_varargs(strdup("testpred2"),
      4, copy_term(c1), copy_term(c2), copy_term(c3), copy_term(c4));
  const struct fmla_t * test_atom3 = mk_fmla_atom_varargs(strdup("testpred3"),
      4, copy_term(c1), copy_term(c2), copy_term(c3), copy_term(c4));
  struct fmlas_t * test_fmlas = mk_fmlas(3, test_atom, test_atom2, test_atom3);
  struct fmlas_t * test_fmlas2 = copy_fmlas(test_fmlas);
  struct fmla_t * test_and2 = mk_fmla_ands(test_fmlas);
  struct fmla_t * test_or2 = mk_fmla_ors(test_fmlas2);

  outbuf = mk_buffer(BUF_SIZE);
  res = fmla_str(test_atom, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test_atom formula (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);
  free_buffer(outbuf);

  outbuf = mk_buffer(BUF_SIZE);
  res = fmla_str(test_and2, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test_and2 formula (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);
  free_buffer(outbuf);

  outbuf = mk_buffer(BUF_SIZE);
  res = fmla_str(test_or, outbuf);
  assert(is_ok_buffer_write_result(res));
  free(res);
  printf("test_or formula (size=%zu, remaining=%zu)\n|%s|\n",
      outbuf->idx, outbuf->buffer_size - outbuf->idx, outbuf->buffer);
  printf("strlen=%zu\n", strlen(outbuf->buffer));
  assert(strlen(outbuf->buffer) + 1 == outbuf->idx);
  free_buffer(outbuf);

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

struct fmla_t *
mk_fmla_quants(const struct terms_t * const vars, struct fmla_t * body)
{
  struct fmla_t * result = body;
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

struct fmlas_t *
copy_fmlas(const struct fmlas_t * fmlas)
{
  struct fmlas_t * result = NULL;

  if (NULL != fmlas) {
    result = malloc(sizeof(struct fmlas_t));
    assert(NULL != fmlas->fmla);
    result->fmla = copy_fmla(fmlas->fmla);
    result->next = copy_fmlas(fmlas->next);
  }

  return result;
}
