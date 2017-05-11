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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define BUF_SIZE 1600

#define VERBOSE printf
#define ERR(...) fflush(stdout); fprintf(stderr, __VA_ARGS__)

#if DEBUG
#define DBG fflush(stdout); printf
#else
#define DBG(...)
#endif // DEBUG

#define DECLARE_LIST_TYPE(TYPE_NAME, FIELD_NAME, ELEMENTS_TY) \
  struct TYPE_NAME { \
    const struct ELEMENTS_TY * FIELD_NAME; \
    const struct TYPE_NAME * next; \
  };

#define DECLARE_MUTABLE_LIST_TYPE(TYPE_NAME, FIELD_NAME, ELEMENTS_TY) \
  struct TYPE_NAME { \
    struct ELEMENTS_TY * FIELD_NAME; \
    struct TYPE_NAME * next; \
  };

#define DECLARE_U8_LIST_LEN(TYPE_NAME) \
  uint8_t len_ ## TYPE_NAME ## _cell(const struct TYPE_NAME ## _t * next);

#define DEFINE_U8_LIST_LEN(TYPE_NAME) \
  uint8_t \
  len_ ## TYPE_NAME ## _cell(const struct TYPE_NAME ## _t * next) \
  { \
    uint8_t result = 0; \
    while (NULL != next) { \
      result++; \
      next = next->next; \
    } \
    return result; \
  }

// FIXME include functions for FREE and STR wrt list.

#define __DECLARE_LIST_MK(NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  RESULT_TYCON LIST_TYPE * mk_ ## NAME ## _cell (const EL_TYPE * const el, const LIST_TYPE * const lst)
#define DECLARE_LIST_MK(NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  __DECLARE_LIST_MK(NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON);
#define DEFINE_LIST_MK(FIELD_NAME, NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  __DECLARE_LIST_MK(NAME, EL_TYPE, LIST_TYPE, RESULT_TYCON) \
  { \
    assert(NULL != el); \
  \
    LIST_TYPE * lsts = malloc(sizeof(LIST_TYPE)); \
    assert(NULL != lsts); \
  \
    lsts->FIELD_NAME = el; \
    lsts->next = lst; \
    return lsts; \
  }

#define __DECLARE_LIST_REV(NAME, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST) \
  TYPE_OP_POST TYPE_NAME * reverse_ ## NAME (TYPE_OP_PRE TYPE_NAME * lst)
#define DECLARE_LIST_REV(NAME, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST) \
  __DECLARE_LIST_REV(NAME, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST);
// FIXME "const" is sticky, in that if TYPE_OP_PRE==const then must be that TYPE_OP_POST==const
#define DEFINE_LIST_REV(NAME, CELL_MAKER, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST) \
  __DECLARE_LIST_REV(NAME, TYPE_OP_PRE, TYPE_NAME, TYPE_OP_POST) \
  { \
    TYPE_OP_POST TYPE_NAME * result = NULL; \
    while (NULL != lst) { \
      assert(NULL != lst->stmt); \
      result = CELL_MAKER(lst->stmt, result); \
      lst = lst->next; \
    } \
    return result; \
  }

#endif /* __TYM_UTIL_H__ */
