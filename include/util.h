/*
 * Useful "definitions" of broad scope.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_UTIL_H__
#define __TYM_UTIL_H__

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "lifted.h"

#define TYM_BUF_SIZE 2600

#define TYM_VERBOSE printf
#define TYM_ERR(...) fflush(stdout); fprintf(stderr, __VA_ARGS__)

#if TYM_DEBUG
#define TYM_DBG fflush(stdout); printf
#else
#define TYM_DBG(...)
#endif // TYM_DEBUG

#define TYM_DECLARE_LIST_TYPE(TYPE_NAME, FIELD_NAME, ELEMENTS_TY) \
  struct TYPE_NAME { \
    const struct ELEMENTS_TY * FIELD_NAME; \
    const struct TYPE_NAME * next; \
  };

// FIXME can factor some code with DECLARE_LIST_TYPE?
#define TYM_DECLARE_MUTABLE_LIST_TYPE(TYPE_NAME, FIELD_NAME, ELEMENTS_TY) \
  struct TYPE_NAME { \
    struct ELEMENTS_TY * FIELD_NAME; \
    struct TYPE_NAME * next; \
  };

#define TYM_DECLARE_U8_LIST_LEN(TYPE_NAME) \
  uint8_t tym_len_ ## TYPE_NAME ## _cell(const struct TYPE_NAME * next);

#define TYM_DEFINE_U8_LIST_LEN(TYPE_NAME) \
  uint8_t \
  tym_len_ ## TYPE_NAME ## _cell(const struct TYPE_NAME * next) \
  { \
    uint8_t result = 0; \
    while (NULL != next) { \
      result++; \
      next = next->next; \
    } \
    return result; \
  }

// FIXME include functions for FREE and STR wrt list.

#define __TYM_DECLARE_LIST_MK(NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  RESULT_TYCON LIST_TYPE * tym_mk_ ## NAME ## _cell (const EL_TYPE * const el, const LIST_TYPE * const lst)
#define TYM_DECLARE_LIST_MK(NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  __TYM_DECLARE_LIST_MK(NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON);
#define __TYM_DEFINE_LIST_MK(FIELD_NAME, NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  { \
    assert(NULL != el); \
  \
    LIST_TYPE * lsts = malloc(sizeof *lsts); \
    assert(NULL != lsts); \
  \
    *lsts = (LIST_TYPE){el, lst}; \
    return lsts; \
  }
#define TYM_DEFINE_LIST_MK(FIELD_NAME, NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  __TYM_DECLARE_LIST_MK(NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  __TYM_DEFINE_LIST_MK(FIELD_NAME, NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON)

#define __TYM_DECLARE_MUTABLE_LIST_MK(NAME, EL_TYPE, LIST_TYPE) \
  LIST_TYPE * tym_mk_ ## NAME ## _cell (EL_TYPE * el, LIST_TYPE * lst)
#define TYM_DECLARE_MUTABLE_LIST_MK(NAME, EL_TYPE, LIST_TYPE) \
  __TYM_DECLARE_MUTABLE_LIST_MK(NAME, EL_TYPE, LIST_TYPE);
#define TYM_DEFINE_MUTABLE_LIST_MK(FIELD_NAME, NAME, EL_TYPE, LIST_TYPE) \
  __TYM_DECLARE_MUTABLE_LIST_MK(NAME, EL_TYPE, LIST_TYPE) \
  __TYM_DEFINE_LIST_MK(FIELD_NAME, NAME, EL_TYPE, LIST_TYPE, /* no const */)

#define __TYM_DECLARE_LIST_REV(NAME, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST) \
  TYPE_OP_POST TYPE_NAME * tym_reverse_ ## NAME (TYPE_OP_PRE TYPE_NAME * lst)
#define TYM_DECLARE_LIST_REV(NAME, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST) \
  __TYM_DECLARE_LIST_REV(NAME, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST);
// FIXME "const" is sticky, in that if TYPE_OP_PRE==const then must be that TYPE_OP_POST==const
#define TYM_DEFINE_LIST_REV(NAME, CELL_MAKER, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST) \
  __TYM_DECLARE_LIST_REV(NAME, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST) \
  { \
    TYPE_OP_POST TYPE_NAME * result = NULL; \
    while (NULL != lst) { \
      assert(NULL != lst->stmt); \
      result = CELL_MAKER(lst->stmt, result); \
      lst = lst->next; \
    } \
    return result; \
  }

#define __TYM_DECLARE_LIST_SHALLOW_FREE(NAME, TYPE_OP_PRE, TYPE_NAME) \
  void tym_shallow_free_ ## NAME (TYPE_OP_PRE TYPE_NAME * lst)
#define TYM_DECLARE_LIST_SHALLOW_FREE(NAME, TYPE_OP_PRE, TYPE_NAME) \
  __TYM_DECLARE_LIST_SHALLOW_FREE(NAME, TYPE_OP_PRE, TYPE_NAME);
#define TYM_DEFINE_LIST_SHALLOW_FREE(NAME, TYPE_OP_PRE, TYPE_NAME) \
  __TYM_DECLARE_LIST_SHALLOW_FREE(NAME, TYPE_OP_PRE, TYPE_NAME) \
  { \
    TYPE_OP_PRE TYPE_NAME * pre_cursor = NULL; \
    while (NULL != lst) { \
      pre_cursor = lst; \
      lst = lst->next; \
      free((void *)pre_cursor); \
    } \
  }

#endif /* __TYM_UTIL_H__ */
