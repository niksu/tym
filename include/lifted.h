/*
 * A type that can contain data values as well as error indicators.
 * Nik Sultana, May 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_LIFTED_H__
#define __TYM_LIFTED_H__

#define TYM_MAYBE_ERROR(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  struct TYPE_NAME { \
    bool is_error; \
    union { \
      ERROR_TYPE nok_value; \
      RESULT_TYPE ok_value; \
    } value; \
  };

#define __TYM_MAYBE_ERROR__IS_OK(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  bool tym_is_ok_ ## TYPE_NAME (struct TYPE_NAME * v)
#define __TYM_MAYBE_ERROR__VAL_OF(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  RESULT_TYPE tym_val_of_ ## TYPE_NAME (struct TYPE_NAME * v)
#define __TYM_MAYBE_ERROR__ERRVAL_OF(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  ERROR_TYPE tym_errval_of_ ## TYPE_NAME (struct TYPE_NAME * v)
#define __TYM_MAYBE_ERROR__MKVAL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  struct TYPE_NAME * tym_mkval_ ## TYPE_NAME (RESULT_TYPE v)
#define __TYM_MAYBE_ERROR__MKERRVAL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  struct TYPE_NAME * tym_mkerrval_ ## TYPE_NAME (ERROR_TYPE v)

#define TYM_MAYBE_ERROR__IS_OK_DECL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__IS_OK(TYPE_NAME, RESULT_TYPE, ERROR_TYPE);
#define TYM_MAYBE_ERROR__VAL_OF_DECL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__VAL_OF(TYPE_NAME, RESULT_TYPE, ERROR_TYPE);
#define TYM_MAYBE_ERROR__ERRVAL_OF_DECL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__ERRVAL_OF(TYPE_NAME, RESULT_TYPE, ERROR_TYPE);
#define TYM_MAYBE_ERROR__MKVAL_DECL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__MKVAL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE);
#define TYM_MAYBE_ERROR__MKERRVAL_DECL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__MKERRVAL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE);

#define TYM_MAYBE_ERROR__IS_OK_DEFN(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__IS_OK(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  { \
    return (!v->is_error); \
  }
#define TYM_MAYBE_ERROR__VAL_OF_DEFN(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__VAL_OF(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  { \
    assert(!v->is_error); \
    return v->value.ok_value; \
  }
#define TYM_MAYBE_ERROR__ERRVAL_OF_DEFN(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__ERRVAL_OF(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  { \
    assert(v->is_error); \
    return v->value.nok_value; \
  }
#define TYM_MAYBE_ERROR__MKVAL_DEFN(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__MKVAL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  { \
    struct TYPE_NAME * result = malloc(sizeof(struct TYPE_NAME)); \
    result->is_error = false; \
    result->value.ok_value = v; \
    return result; \
  }
#define TYM_MAYBE_ERROR__MKERRVAL_DEFN(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  __TYM_MAYBE_ERROR__MKERRVAL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE) \
  { \
    struct TYPE_NAME * value = malloc(sizeof(struct TYPE_NAME)); \
    value->is_error = true; \
    value->value.nok_value = v; \
    return value; \
  }

//Error handler, explains what's wrong (via function pointer applied to
//situation-specific data) and terminates.
#define __TYM_ERROR_CHECK(TYPE_NAME, RESULT_TYPE, ERROR_TYPE, F) \
  void error_check_ ## TYPE_NAME (struct TYPE_NAME * v, void (*F)(void *), void * ctxt)
#define TYM_ERROR_CHECK_DECL(TYPE_NAME, RESULT_TYPE, ERROR_TYPE, F) \
  __TYM_ERROR_CHECK(TYPE_NAME, RESULT_TYPE, ERROR_TYPE, F);
#define TYM_ERROR_CHECK_DEFN(TYPE_NAME, RESULT_TYPE, ERROR_TYPE, F) \
  __TYM_ERROR_CHECK(TYPE_NAME, RESULT_TYPE, ERROR_TYPE, F) \
  { \
    if (v->is_error) { \
      F(ctxt); \
    }; \
  }

#endif /* __TYM_LIFTED_H__ */
