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


This file: Representation of formulas.
*/

#include <assert.h>

#include "ast.h"
#include "formula.h"
#include "module_tests.h"
#include "util.h"

uint8_t TymMaxVarWidth = 10;
char * TYM_UNIVERSE_TY = "Universe";

TYM_DEFINE_LIST_REV(fmla, fmlas, tym_mk_fmla_cell, , struct TymFmlas, )

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_fmla_junction_str(struct TymFmla ** fmla, struct TymBufferInfo * dst);
static struct TymFmlas * tym_copy_fmlas(const struct TymFmlas *);
static struct TymFmlas * filter_before_juncts(struct TymFmlas * fmlas, bool is_and_behaviour);

struct TymFmla *
tym_mk_fmla_const(bool b)
{
  struct TymFmla * result = malloc(sizeof *result);
  assert(NULL != result);

  result->kind = FMLA_CONST;
  result->param.const_value = b;

  return result;
}

struct TymFmla *
tym_mk_fmla_atom(const TymStr * pred_name, uint8_t arity, struct TymTerm ** predargs)
{
  struct TymFmlaAtom * result_content = malloc(sizeof *result_content);
  assert(NULL != result_content);
  struct TymFmla * result = malloc(sizeof *result);
  assert(NULL != result);

  result->kind = FMLA_ATOM;
  result->param.atom = result_content;

  result_content->pred_name = pred_name;
  result_content->pred_const =
    tym_mk_term(TYM_CONST, TYM_STR_DUPLICATE(pred_name));
  result_content->arity = arity;
  result_content->predargs = predargs;

  return result;
}

struct TymFmla *
tym_mk_fmla_atom_varargs(const TymStr * pred_name, unsigned int arity, ...)
{
  struct TymTerm ** args = NULL;

  va_list varargs;
  va_start(varargs, arity);
  if (arity > 0) {
    args = malloc(sizeof *args * arity);
    for (unsigned int i = 0; i < arity; i++) {
      args[i] = va_arg(varargs, struct TymTerm *);
    }
  }
  va_end(varargs);

  return tym_mk_fmla_atom(pred_name, (uint8_t)arity, args);
}

struct TymFmla *
tym_mk_fmla_quant(const enum TymFmlaKind quant, const TymStr * bv, struct TymFmla * body)
{
  assert(FMLA_EX == quant || FMLA_ALL == quant);
  assert(NULL != bv);
  assert(NULL != body);
  struct TymFmlaQuant * result_content = malloc(sizeof *result_content);
  struct TymFmla * result = malloc(sizeof *result);
  result_content->bv = bv;
  result_content->body = body;
  *result = (struct TymFmla){.kind = quant, .param.quant = result_content};
  return result;
}

struct TymFmla *
tym_mk_fmla_not(struct TymFmla * subfmla)
{
  struct TymFmla ** result_content = malloc(sizeof *result_content * 1);
  struct TymFmla * result = malloc(sizeof *result);
  result_content[0] = subfmla;
  *result = (struct TymFmla){.kind = FMLA_NOT, .param.args = result_content};
  return result;
}

struct TymFmla *
tym_mk_fmla_if(struct TymFmla * subfmlaL, struct TymFmla * subfmlaR)
{
  if (tym_fmla_is_const(subfmlaL)) {
    if (tym_fmla_as_const(subfmlaL)) {
      tym_free_fmla(subfmlaL);
      return subfmlaR;
    } else {
      tym_free_fmla(subfmlaL);
      tym_free_fmla(subfmlaR);
      return tym_mk_fmla_const(true);
    }
  } else {
    if (tym_fmla_is_const(subfmlaR)) {
      if (tym_fmla_as_const(subfmlaR)) {
        tym_free_fmla(subfmlaR);
        return subfmlaL;
      } else {
        tym_free_fmla(subfmlaR);
        return tym_mk_fmla_not(subfmlaL);
      }
    }
  }

  struct TymFmla ** result_content = malloc(sizeof *result_content * 3);
  struct TymFmla * result = malloc(sizeof *result);
  result->kind = FMLA_IF;
  result_content[0] = subfmlaL;
  result_content[1] = subfmlaR;
  result_content[2] = NULL;
  result->param.args = result_content;
  return result;
}

struct TymFmla *
tym_mk_fmla_iff(struct TymFmla * subfmlaL, struct TymFmla * subfmlaR)
{
  if (tym_fmla_is_const(subfmlaL)) {
    if (tym_fmla_as_const(subfmlaL)) {
      if (tym_fmla_is_const(subfmlaR)) {
        tym_free_fmla(subfmlaL);
        return subfmlaR;
      }
    } else {
      if (tym_fmla_is_const(subfmlaR)) {
        if (tym_fmla_as_const(subfmlaR)) {
          tym_free_fmla(subfmlaR);
          return subfmlaL;
        } else {
          tym_free_fmla(subfmlaL);
          tym_free_fmla(subfmlaR);
          return tym_mk_fmla_const(true);
        }
      }
    }
  }

  struct TymFmla ** result_content = malloc(sizeof *result_content * 3);
  struct TymFmla * result = malloc(sizeof *result);
  result->kind = FMLA_IFF;
  result_content[0] = subfmlaL;
  result_content[1] = subfmlaR;
  result_content[2] = NULL;
  result->param.args = result_content;
  return result;
}

struct TymFmla *
tym_mk_fmla_and(struct TymFmla * subfmlaL, struct TymFmla * subfmlaR)
{
  if (tym_fmla_is_const(subfmlaL)) {
    if (tym_fmla_as_const(subfmlaL)) {
      if (tym_fmla_is_const(subfmlaR)) {
        tym_free_fmla(subfmlaR);
        return subfmlaL;
      } else {
        tym_free_fmla(subfmlaL);
        return subfmlaR;
      }
    } else {
      tym_free_fmla(subfmlaR);
      return subfmlaL;
    }
  } else {
    if (tym_fmla_is_const(subfmlaR)) {
      if (tym_fmla_as_const(subfmlaR)) {
        tym_free_fmla(subfmlaR);
        return subfmlaL;
      } else {
        tym_free_fmla(subfmlaL);
        return subfmlaR;
      }
    }
  }

  struct TymFmla ** result_content = malloc(sizeof *result_content * 3);
  struct TymFmla * result = malloc(sizeof *result);
  result->kind = FMLA_AND;
  result_content[0] = subfmlaL;
  result_content[1] = subfmlaR;
  result_content[2] = NULL;
  result->param.args = result_content;
  return result;
}

struct TymFmla *
tym_mk_fmla_or(struct TymFmla * subfmlaL, struct TymFmla * subfmlaR)
{
  if (tym_fmla_is_const(subfmlaL)) {
    if (tym_fmla_as_const(subfmlaL)) {
      tym_free_fmla(subfmlaR);
      return subfmlaL;
    } else {
      tym_free_fmla(subfmlaL);
      return subfmlaR;
    }
  } else {
    if (tym_fmla_is_const(subfmlaR)) {
      if (tym_fmla_as_const(subfmlaR)) {
        tym_free_fmla(subfmlaL);
        return subfmlaR;
      } else {
        tym_free_fmla(subfmlaR);
        return subfmlaL;
      }
    }
  }

  struct TymFmla ** result_content = malloc(sizeof *result_content * 3);
  struct TymFmla * result = malloc(sizeof *result);
  result->kind = FMLA_OR;
  result_content[0] = subfmlaL;
  result_content[1] = subfmlaR;
  result_content[2] = NULL;
  result->param.args = result_content;
  return result;
}

void transfer_cell(struct TymFmlas ** from_fmlas, struct TymFmlas ** to_fmlas);
void chomp_cell(struct TymFmlas ** fmlas);

void
chomp_cell(struct TymFmlas ** fmlas)
{
  if (NULL != *fmlas) {
    struct TymFmlas * next = (*fmlas)->next;
    free(*fmlas);
    *fmlas = next;
  }
}

void
transfer_cell(struct TymFmlas ** from_fmlas, struct TymFmlas ** to_fmlas)
{
  if (NULL == *to_fmlas) {
    *to_fmlas = tym_mk_fmla_cell((*from_fmlas)->fmla, NULL);
    chomp_cell(from_fmlas);
  } else {
    *to_fmlas = tym_mk_fmla_cell((*from_fmlas)->fmla, *to_fmlas); // NOTE reversed
    chomp_cell(from_fmlas);
  }
}

struct TymFmlas *
filter_before_juncts(struct TymFmlas * fmlas, bool is_and_behaviour)
{
  struct TymFmlas * new_fmlas = NULL;
  bool saw_false = false;
  bool saw_true = false;
  if (NULL != fmlas) {
    struct TymFmlas * cursor = fmlas;
    while (NULL != cursor) {
      if (tym_fmla_is_const(cursor->fmla)) {
        if (tym_fmla_as_const(cursor->fmla)) {
          if (is_and_behaviour) {
            chomp_cell(&cursor);
          } else {
            saw_true = true;
            break;
          }
        } else {
          if (is_and_behaviour) {
            saw_false = true;
            break;
          } else {
            chomp_cell(&cursor);
          }
        }
      } else {
        transfer_cell(&cursor, &new_fmlas);
      }
    }

    if (saw_false && is_and_behaviour) {
      // fmlas degenerates to "false"
      if (NULL != new_fmlas) {
        tym_free_fmlas(new_fmlas);
      }
      if (NULL != cursor) {
        tym_free_fmlas(cursor);
      }
      new_fmlas = tym_mk_fmla_cell(tym_mk_fmla_const(false), NULL);
    } else if (saw_true && !is_and_behaviour) {
      // fmlas degenerates to "true"
      if (NULL != new_fmlas) {
        tym_free_fmlas(new_fmlas);
      }
      if (NULL != cursor) {
        tym_free_fmlas(cursor);
      }
      new_fmlas = tym_mk_fmla_cell(tym_mk_fmla_const(true), NULL);
    } else {
      assert(!(saw_false || saw_true));
    }
  }

  struct TymFmlas * reversed = tym_reverse_fmlas(new_fmlas);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  struct TymFmlas * pre_cursor = NULL;
  struct TymFmlas * cursor = new_fmlas;
  while (NULL != cursor) {
    pre_cursor = cursor;
    cursor = cursor->next;
    free((void *)pre_cursor);
  }
#pragma GCC diagnostic pop
  return reversed;
}

struct TymFmla *
tym_mk_fmla_ands(struct TymFmlas * fmlas)
{
  fmlas = filter_before_juncts(fmlas, true);
  struct TymFmla * result;
  if (NULL == fmlas) {
    result = tym_mk_fmla_const(true);
  } else {
    if (NULL == fmlas->next) {
      result = fmlas->fmla;
      free(fmlas);
    } else {
      unsigned int no_fmlas = TYM_LIST_LEN(TymFmlas)(fmlas);
      struct TymFmla ** result_content = malloc(sizeof *result_content *
        (no_fmlas + 1));
      result = malloc(sizeof *result);
      result->kind = FMLA_AND;
      struct TymFmlas * cursor = fmlas;
      for (unsigned int i = 0; i < no_fmlas; i++) {
        result_content[i] = cursor->fmla;
        cursor = cursor->next;
        free(fmlas);
        fmlas = cursor;
      }
      result_content[no_fmlas] = NULL;
      result->param.args = result_content;
    }
  }
  return result;
}

struct TymFmla *
tym_mk_fmla_ors(struct TymFmlas * fmlas)
{
  fmlas = filter_before_juncts(fmlas, false);
  struct TymFmla * result;
  if (NULL == fmlas) {
    result = tym_mk_fmla_const(false);
  } else {
    if (NULL == fmlas->next) {
      result = fmlas->fmla;
      free(fmlas);
    } else {
      unsigned int no_fmlas = TYM_LIST_LEN(TymFmlas)(fmlas);
      struct TymFmla ** result_content = malloc(sizeof *result_content *
        (no_fmlas + 1));
      result = malloc(sizeof *result);
      result->kind = FMLA_OR;
      struct TymFmlas * cursor = fmlas;
      for (unsigned int i = 0; i < no_fmlas; i++) {
        result_content[i] = cursor->fmla;
        cursor = cursor->next;
        free(fmlas);
        fmlas = cursor;
      }
      result_content[no_fmlas] = NULL;
      result->param.args = result_content;
    }
  }
  return result;
}

struct TymFmla *
tym_mk_fmla_imply(struct TymFmla * antecedent, struct TymFmla * consequent)
{
  struct TymFmla * subfmla = tym_mk_fmla_not(antecedent);
  struct TymFmla * result = tym_mk_fmla_or(subfmla, consequent);
  return result;
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_fmla_atom_str(struct TymFmlaAtom * at, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_buf_strcpy(dst, tym_decode_str(at->pred_name));
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.

  for (int i = 0; i < at->arity; i++) {
    if (tym_have_space(dst, 1)) {
      tym_unsafe_buffer_char(dst, ' ');
    } else {
      return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
    }

    res = tym_term_str(at->predargs[i], dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.
  }

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_fmla_quant_str(struct TymFmlaQuant * quant, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);

  if (tym_have_space(dst, 2)) {
    tym_unsafe_buffer_str(dst, "((");
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_buf_strcpy(dst, tym_decode_str(quant->bv));
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.

  res = tym_buf_strcpy(dst, TYM_UNIVERSE_TY);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.

  tym_unsafe_buffer_str(dst, ") ");
  tym_unsafe_dec_idx(dst, 1); // chomp the trailing \0.

  res = tym_fmla_str(quant->body, dst);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);

  return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_fmla_junction_str(struct TymFmla ** fmla, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);

  while (NULL != *fmla) {
    struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res =
      tym_fmla_str(*fmla, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    fmla++;
  }

  tym_safe_buffer_replace_last(dst, '\0');

  return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_fmla_str(const struct TymFmla * fmla, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);

  const size_t fmla_sz = tym_fmla_size(fmla);
  if (fmla_sz > 1) {
    if (tym_have_space(dst, 1)) {
      tym_unsafe_buffer_char(dst, '(');
    } else {
      return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
    }
  }

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  switch (fmla->kind) {
  case FMLA_CONST:
    if (fmla->param.const_value) {
      res = tym_buf_strcpy(dst, "true");
    } else {
      res = tym_buf_strcpy(dst, "false");
    }
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  case FMLA_ATOM:
    res = tym_fmla_atom_str(fmla->param.atom, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  case FMLA_AND:
    res = tym_buf_strcpy(dst, "and");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = tym_fmla_junction_str(fmla->param.args, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  case FMLA_OR:
    res = tym_buf_strcpy(dst, "or");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = tym_fmla_junction_str(fmla->param.args, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  case FMLA_NOT:
    res = tym_buf_strcpy(dst, "not");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = tym_fmla_str(fmla->param.args[0], dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  case FMLA_EX:
    res = tym_buf_strcpy(dst, "exists");
    error_check_TymBufferWriteResult(res, tym_buff_error_msg, dst);
    free(res);
    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = tym_fmla_quant_str(fmla->param.quant, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  case FMLA_ALL:
    res = tym_buf_strcpy(dst, "forall");
    error_check_TymBufferWriteResult(res, tym_buff_error_msg, dst);
    free(res);
    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = tym_fmla_quant_str(fmla->param.quant, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  case FMLA_IF:
    res = tym_buf_strcpy(dst, "=>");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = tym_fmla_junction_str(fmla->param.args, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  case FMLA_IFF:
    res = tym_buf_strcpy(dst, "=");
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    tym_safe_buffer_replace_last(dst, ' '); // replace the trailing \0.
    res = tym_fmla_junction_str(fmla->param.args, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);
    break;
  default:
    return tym_mkerrval_TymBufferWriteResult(NON_BUFF_ERROR);
  }

  if (fmla_sz > 1) {
    tym_safe_buffer_replace_last(dst, ')'); // replace the trailing \0.
  } else {
    tym_unsafe_dec_idx(dst, 1);
  }

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

TYM_DEFINE_MUTABLE_LIST_MK(fmla, fmla, struct TymFmla, struct TymFmlas)
TYM_DEFINE_LIST_LEN(TymFmlas, , struct TymFmlas)

struct TymSymGen *
tym_mk_sym_gen(const TymStr * prefix)
{
  struct TymSymGen * result = malloc(sizeof *result);
  result->prefix = prefix;
  result->index = 0;
  return result;
}

struct TymSymGen *
tym_copy_sym_gen(const struct TymSymGen * const cp_orig)
{
  struct TymSymGen * result = malloc(sizeof *result);
  result->prefix = TYM_STR_DUPLICATE(cp_orig->prefix);
  result->index = cp_orig->index;
  return result;
}

const TymStr *
tym_mk_new_var(struct TymSymGen * vg)
{
  size_t i = tym_len_str(vg->prefix);
  char * result = malloc(i + 1 + TymMaxVarWidth);
  strcpy(result, tym_decode_str(vg->prefix));
  snprintf(result + i, TymMaxVarWidth, "%lu", vg->index);
  vg->index += 1;
  return tym_encode_str(result);
}

bool
tym_fmla_is_atom(const struct TymFmla * fmla)
{
  switch (fmla->kind) {
  case FMLA_ATOM:
    return true;
  default:
    return false;
  }
}

struct TymFmlaAtom *
tym_fmla_as_atom(const struct TymFmla * fmla)
{
  if (tym_fmla_is_atom(fmla)) {
    return fmla->param.atom;
  } else {
    return NULL;
  }
}

bool
tym_fmla_is_const(const struct TymFmla * fmla)
{
  switch (fmla->kind) {
  case FMLA_CONST:
    return true;
  default:
    return false;
  }
}

bool
tym_fmla_as_const(const struct TymFmla * fmla)
{
  if (tym_fmla_is_const(fmla)) {
    return fmla->param.const_value;
  } else {
    assert(false);
  }
}

const struct TymFmla *
tym_mk_abstract_vars(const struct TymFmla * at, struct TymSymGen * vg, struct TymValuation ** v)
{
  struct TymFmlaAtom * atom = tym_fmla_as_atom(at);
  assert(NULL != atom);

  struct TymTerm ** var_args_T = NULL;

  if (atom->arity > 0) {
    var_args_T = malloc(sizeof *var_args_T * atom->arity);
    const TymStr ** var_args = malloc(sizeof *var_args * atom->arity);
    *v = NULL;

    struct TymValuation * v_cursor;
    for (int i = 0; i < atom->arity; i++) {
      if (0 == i) {
        *v = malloc(sizeof **v);
        v_cursor = *v;
      } else {
        assert(NULL != v_cursor);
        v_cursor->next = malloc(sizeof *v_cursor->next);
        v_cursor = v_cursor->next;
      }

      v_cursor->val = tym_copy_term(atom->predargs[i]);

      v_cursor->var = tym_mk_new_var(vg);
      var_args[i] = v_cursor->var;
      var_args_T[i] =
        tym_mk_term(TYM_VAR, TYM_STR_DUPLICATE(v_cursor->var));

      v_cursor->next = NULL;
    }

    free(var_args);
  }

  return tym_mk_fmla_atom(TYM_STR_DUPLICATE(atom->pred_name),
      atom->arity, var_args_T);
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_valuation_str(struct TymValuation * v, struct TymBufferInfo * dst)
{
  size_t initial_idx = tym_buffer_len(dst);

  struct TymValuation * v_cursor = v;

  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = NULL;

  while (NULL != v_cursor) {
    res = tym_buf_strcpy(dst, tym_decode_str(v_cursor->var));
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    tym_safe_buffer_replace_last(dst, '='); // replace the trailing \0.

    res = tym_term_str(v_cursor->val, dst);
    assert(tym_is_ok_TymBufferWriteResult(res));
    free(res);

    v_cursor = v_cursor->next;

    if (NULL != v_cursor) {
      tym_safe_buffer_replace_last(dst, ','); // replace the trailing \0.

      if (tym_have_space(dst, 1)) {
        tym_unsafe_buffer_char(dst, ' ');
      } else {
        return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
      }
    }
  }

  if (tym_have_space(dst, 1)) {
    tym_unsafe_buffer_char(dst, '\0');
    return tym_mkval_TymBufferWriteResult(tym_buffer_len(dst) - initial_idx);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

void
tym_free_fmla_atom(struct TymFmlaAtom * at)
{
  tym_free_str(at->pred_name);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  tym_free_term((struct TymTerm *)at->pred_const);
#pragma GCC diagnostic pop

  for (int i = 0; i < at->arity; i++) {
    tym_free_term(at->predargs[i]);
  }

  if (NULL != at->predargs) {
    assert(at->arity >= 0);
    free(at->predargs);
  } else {
    assert(0 == at->arity);
  }

  free(at);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_fmla_quant(struct TymFmlaQuant * q)
{
  assert (NULL != q->bv);
  tym_free_str(q->bv);

  assert (NULL != q->body);
  tym_free_fmla(q->body);

  free(q);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_fmla(const struct TymFmla * fmla)
{
  switch (fmla->kind) {
  case FMLA_CONST:
    // Nothing internal to free. We simply free "fmla" at the end.
    break;
  case FMLA_ATOM:
    tym_free_fmla_atom(fmla->param.atom);
    break;
  case FMLA_AND:
  case FMLA_OR:
  case FMLA_IF:
  case FMLA_IFF:
    for (int i = 0; NULL != fmla->param.args[i]; i++) {
      tym_free_fmla(fmla->param.args[i]);
    }
    free(fmla->param.args);
    break;
  case FMLA_NOT:
    tym_free_fmla(fmla->param.args[0]);
    free(fmla->param.args);
    break;
  case FMLA_EX:
  case FMLA_ALL:
    tym_free_fmla_quant(fmla->param.quant);
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
tym_free_fmlas(const struct TymFmlas * fmlas)
{
  assert(NULL != fmlas->fmla);
  tym_free_fmla(fmlas->fmla);
  if (NULL != fmlas->next) {
    tym_free_fmlas(fmlas->next);
  }

  free((void *)fmlas);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
tym_free_sym_gen(struct TymSymGen * vg)
{
  assert(NULL != vg->prefix);
  tym_free_str(vg->prefix);

  free(vg);
}
#pragma GCC diagnostic pop

void
tym_free_valuation(struct TymValuation * v)
{
  assert(NULL != v);

  assert(NULL != v->var);
  tym_free_str(v->var);

  assert(NULL != v->val);
  tym_free_term(v->val);

  if (NULL != v->next) {
    tym_free_valuation(v->next);
  }

  free(v);
}

struct TymFmlas *
tym_mk_fmlas(unsigned int no_fmlas, ...)
{
  va_list varargs;
  va_start(varargs, no_fmlas);
  struct TymFmlas * result = NULL;
  for (unsigned int i = 0; i < no_fmlas; i++) {
    struct TymFmla * cur_fmla = va_arg(varargs, struct TymFmla *);
    assert(NULL != cur_fmla);
    result = tym_mk_fmla_cell(tym_copy_fmla(cur_fmla), result);
  }
  va_end(varargs);
  return result;
}

struct TymFmla *
tym_copy_fmla(const struct TymFmla * const fmla)
{
  struct TymFmla * result = NULL;

  struct TymTerm ** predargs_copy = NULL;

  switch (fmla->kind) {
  case FMLA_CONST:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct TymFmla *)tym_mk_fmla_const(fmla->param.const_value);
#pragma GCC diagnostic pop
    break;
  case FMLA_ATOM:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    predargs_copy = NULL;

    if (fmla->param.atom->arity > 0) {
      predargs_copy = malloc(sizeof *predargs_copy * fmla->param.atom->arity);
      for (int i = 0; i < fmla->param.atom->arity; i++) {
        predargs_copy[i] = tym_copy_term(fmla->param.atom->predargs[i]);
      }
    }

    result = tym_mk_fmla_atom(TYM_STR_DUPLICATE(fmla->param.atom->pred_name),
        fmla->param.atom->arity, predargs_copy);
#pragma GCC diagnostic pop
    break;
  case FMLA_AND:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    {
      int i = 0;
      struct TymFmlas * fmlas = NULL;
      while (NULL != fmla->param.args[i]) {
        fmlas = tym_mk_fmla_cell(tym_copy_fmla(fmla->param.args[i]), fmlas);
        i++;
      }
      result = (struct TymFmla *)tym_mk_fmla_ands(fmlas);
    }
#pragma GCC diagnostic pop
    break;
  case FMLA_OR:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    {
      int i = 0;
      struct TymFmlas * fmlas = NULL;
      while (NULL != fmla->param.args[i]) {
        fmlas = tym_mk_fmla_cell(tym_copy_fmla(fmla->param.args[i]), fmlas);
        i++;
      }
      result = (struct TymFmla *)tym_mk_fmla_ors(fmlas);
    }
#pragma GCC diagnostic pop
    break;
  case FMLA_NOT:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result = (struct TymFmla *)tym_mk_fmla_not(tym_copy_fmla(fmla->param.args[0]));
#pragma GCC diagnostic pop
    break;
  case FMLA_EX:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    result =
      (struct TymFmla *)tym_mk_fmla_quant(FMLA_EX, TYM_STR_DUPLICATE(fmla->param.quant->bv),
        tym_copy_fmla(fmla->param.quant->body));
#pragma GCC diagnostic pop
    break;
  default:
    assert(false);
    break;
  }

  return result;
}

void
tym_test_formula(void)
{
  printf("***test_formula***\n");
  struct TymTerm ** args = malloc(sizeof *args * 2);
  args[0] = tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("arg0"));
  args[1] = tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("arg1"));

  for (int i = 0; i < 2; i++) {
    TYM_DBG("  :%s\n", tym_decode_str(args[i]->identifier));
  }

  struct TymFmla * test_atom = tym_mk_fmla_atom(TYM_CSTR_DUPLICATE("atom"), 2, args);

  for (int i = 0; i < 2; i++) {
    TYM_DBG("  ;%s\n", tym_decode_str(test_atom->param.atom->predargs[i]->identifier));
  }

  struct TymFmla * test_not = tym_mk_fmla_not(tym_copy_fmla(test_atom));
  struct TymFmla * test_and = tym_mk_fmla_and(tym_copy_fmla(test_not), tym_copy_fmla(test_atom));
  struct TymFmla * test_or = tym_mk_fmla_or(tym_copy_fmla(test_not), tym_copy_fmla(test_and));
  struct TymFmla * test_quant = tym_mk_fmla_quant(FMLA_EX, TYM_CSTR_DUPLICATE("x"),
      tym_copy_fmla(test_or));

  struct TymBufferInfo * outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * res = tym_fmla_str(test_quant, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  TYM_DBG_BUFFER(outbuf, "test formula")
  tym_free_buffer(outbuf);
  tym_free_fmla(test_and);
  tym_free_fmla(test_atom);
  tym_free_fmla(test_not);
  tym_free_fmla(test_quant);

  struct TymTerm * c1 = tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("ta1"));
  struct TymTerm * c2 = tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("ta2"));
  struct TymTerm * c3 = tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("ta3"));
  struct TymTerm * c4 = tym_mk_term(TYM_CONST, TYM_CSTR_DUPLICATE("ta4"));
  test_atom = tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE("testpred1"), 4, c1, c2, c3, c4);
  const struct TymFmla * test_atom2 = tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE("testpred2"),
      4, tym_copy_term(c1), tym_copy_term(c2), tym_copy_term(c3), tym_copy_term(c4));
  const struct TymFmla * test_atom3 = tym_mk_fmla_atom_varargs(TYM_CSTR_DUPLICATE("testpred3"),
      4, tym_copy_term(c1), tym_copy_term(c2), tym_copy_term(c3), tym_copy_term(c4));
  struct TymFmlas * test_fmlas = tym_mk_fmlas(3, test_atom, test_atom2, test_atom3);
  struct TymFmlas * test_fmlas2 = tym_copy_fmlas(test_fmlas);
  struct TymFmla * test_and2 = tym_mk_fmla_ands(test_fmlas);
  struct TymFmla * test_or2 = tym_mk_fmla_ors(test_fmlas2);

  outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  res = tym_fmla_str(test_atom, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  TYM_DBG_BUFFER(outbuf, "test_atom formula")
  tym_free_buffer(outbuf);

  outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  res = tym_fmla_str(test_and2, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  TYM_DBG_BUFFER(outbuf, "test_and2 formula")
  tym_free_buffer(outbuf);

  outbuf = tym_mk_buffer(TYM_BUF_SIZE);
  res = tym_fmla_str(test_or, outbuf);
  assert(tym_is_ok_TymBufferWriteResult(res));
  free(res);
  TYM_DBG_BUFFER(outbuf, "test_or formula")
  tym_free_buffer(outbuf);

  tym_free_fmla(test_atom);
  tym_free_fmla(test_atom2);
  tym_free_fmla(test_atom3);
  tym_free_fmla(test_and2);
  tym_free_fmla(test_or);
  tym_free_fmla(test_or2);
}

struct TymTerms *
tym_filter_var_values(struct TymValuation * const v)
{
  struct TymTerms * result = NULL;
  struct TymValuation * cursor = v;
  while (NULL != cursor) {
    if (TYM_VAR == cursor->val->kind) {
      struct TymTerm * t_copy = tym_copy_term(cursor->val);
      result = tym_mk_term_cell(t_copy, result);
    }
    cursor = cursor->next;
  }
  return result;
}

struct TymFmla *
tym_mk_fmla_quants(const enum TymFmlaKind quant, const struct TymTerms * const vars, struct TymFmla * body)
{
  assert(FMLA_EX == quant || FMLA_ALL == quant);
  struct TymFmla * result = body;
  const struct TymTerms * cursor = vars;
  while (NULL != cursor) {
    assert(TYM_VAR == cursor->term->kind);
    struct TymFmla * pre_result =
      tym_mk_fmla_quant(quant, TYM_STR_DUPLICATE(cursor->term->identifier), result);
    result = pre_result;

    cursor = cursor->next;
  }

  return result;
}

size_t
tym_valuation_len(const struct TymValuation * v)
{
  size_t l = 0;
  while (NULL != v) {
    l++;
    v = v->next;
  }
  return l;
}

struct TymTerms *
tym_arguments_of_atom(struct TymFmlaAtom * fmla)
{
  struct TymTerms * result = NULL;
  for (int i = fmla->arity - 1; i >= 0; i--) {
    result = tym_mk_term_cell(tym_copy_term(fmla->predargs[i]), result);
  }
  return result;
}

size_t
tym_fmla_size(const struct TymFmla * const fmla)
{
  size_t result = 0;
  switch (fmla->kind) {
  case FMLA_CONST:
    result = 1;
    break;
  case FMLA_ATOM:
    result = 1u + fmla->param.atom->arity;
    break;
  case FMLA_AND:
  case FMLA_OR:
  case FMLA_IFF:
    result = 1;
    int i = 0;
    while (NULL != fmla->param.args[i]) {
      result += tym_fmla_size(fmla->param.args[i]);
      i += 1;
    }
    break;
  case FMLA_NOT:
    result = 1 + tym_fmla_size(fmla->param.args[0]);
    break;
  case FMLA_EX:
  case FMLA_ALL:
    result = 1 + tym_fmla_size(fmla->param.quant->body);
    break;
  case FMLA_IF:
    result = 1 + tym_fmla_size(fmla->param.args[0]) + tym_fmla_size(fmla->param.args[1]);
    break;
  default:
    assert(false);
    break;
  }

  return result;
}

struct TymTerms *
tym_consts_in_fmla(const struct TymFmla * fmla, struct TymTerms * acc, bool with_pred_const)
{
  struct TymTerms * result = NULL;
  int idx;
  switch (fmla->kind) {
  case FMLA_CONST:
    result = acc;
    break;
  case FMLA_ATOM:
    result = acc;

    // We add fmla->param.atom->pred_const to result (although pred_const
    // doesn't appear in the Herbrand Universe) otherwise the subsumption check
    // wouldn't work well.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    if (with_pred_const) {
      // FIXME pass immutable elements to tym_mk_term_cell?
      result = tym_mk_term_cell((struct TymTerm *)fmla->param.atom->pred_const, result);
    }
#pragma GCC diagnostic pop

    for (int i = 0; i < fmla->param.atom->arity; i++) {
      struct TymTerm * t = fmla->param.atom->predargs[i];
      if (TYM_CONST == t->kind) {
        result = tym_mk_term_cell(t, result);
      }
    }
    break;
  case FMLA_AND:
  case FMLA_OR:
  case FMLA_IFF:
    idx = 0;
    while (NULL != fmla->param.args[idx]) {
      acc = tym_consts_in_fmla(fmla->param.args[idx], acc, true);
      idx += 1;
    }
    result = acc;
    break;
  case FMLA_NOT:
    result = tym_consts_in_fmla(fmla->param.args[0], acc, true);
    break;
  case FMLA_EX:
    result = tym_consts_in_fmla(fmla->param.quant->body, acc, true);
    break;
  case FMLA_ALL:
    result = tym_consts_in_fmla(fmla->param.quant->body, acc, true);
    break;
  case FMLA_IF:
    acc = tym_consts_in_fmla(fmla->param.args[0], acc, true);
    result = tym_consts_in_fmla(fmla->param.args[1], acc, true);
    break;
  default:
    assert(false);
    break;
  }
  return result;
}

static struct TymFmlas *
tym_copy_fmlas(const struct TymFmlas * fmlas)
{
  struct TymFmlas * result = NULL;

  if (NULL != fmlas) {
    result = malloc(sizeof *result);
    assert(NULL != fmlas->fmla);
    result->fmla = tym_copy_fmla(fmlas->fmla);
    result->next = tym_copy_fmlas(fmlas->next);
  }

  return result;
}
