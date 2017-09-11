/*
 * Buffer management.
 * Nik Sultana, May 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef __TYM_BUFFER_H__
#define __TYM_BUFFER_H__

#include <stdbool.h>
#include <string.h>

#include "string_idx.h"
#include "util.h"

struct TymBufferInfo;

struct TymBufferInfo * tym_mk_buffer(const size_t buffer_size);
const char * tym_buffer_contents(struct TymBufferInfo * buf);
size_t tym_buffer_len(struct TymBufferInfo * buf);
size_t tym_buffer_size(struct TymBufferInfo * buf);
void tym_reset_buffer(struct TymBufferInfo * buf);
void tym_free_buffer(struct TymBufferInfo * buf);
bool tym_have_space(struct TymBufferInfo * buf, size_t n);
void tym_unsafe_buffer_char(struct TymBufferInfo * buf, char c);
void tym_unsafe_buffer_str(struct TymBufferInfo * buf, char * s);
void tym_unsafe_dec_idx(struct TymBufferInfo * buf, size_t n);
void tym_safe_buffer_replace_last(struct TymBufferInfo * buf, char c);

enum TymBufferErrors {NON_BUFF_ERROR, BUFF_ERR_OVERFLOW};

TYM_MAYBE_ERROR(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__IS_OK_DECL(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__VAL_OF_DECL(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__ERRVAL_OF_DECL(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__MKVAL_DECL(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__MKERRVAL_DECL(TymBufferWriteResult, size_t, enum TymBufferErrors)

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) * tym_buf_strcpy(struct TymBufferInfo * dst, const char * src);

void tym_buff_error_msg(void * x);
TYM_ERROR_CHECK_DECL(TymBufferWriteResult, size_t, enum TymBufferErrors, buff_error_msg)

#if TYM_DEBUG
#define TYM_DBG_BUFFER(buffvar, txt) \
  TYM_DBG("%s (size=%zu, remaining=%zu)\n|%s|\n", \
    txt, \
    tym_buffer_len(buffvar), \
    tym_buffer_size(buffvar) - tym_buffer_len(buffvar), \
    tym_buffer_contents(buffvar)); \
  TYM_DBG("%s strlen=%zu\n", \
    txt, \
    strlen(tym_buffer_contents(buffvar))); \
    assert(((0 == strlen(tym_buffer_contents(buffvar))) && \
            (0 == tym_buffer_len(buffvar))) || \
           strlen(tym_buffer_contents(buffvar)) + 1 == tym_buffer_len(buffvar));
#define TYM_DBG_BUFFER_PRINT(buffvar, txt) \
  TYM_DBG("%s: %s\n", txt,  tym_buffer_contents(buffvar));
#define TYM_DBG_BUFFER_PRINT_ENCLOSE(buffvar, txt, txtend) \
  TYM_DBG("%s%s%s\n", txt,  tym_buffer_contents(buffvar), txtend);
#else
#define TYM_DBG_BUFFER(buffvar, txt)
#define TYM_DBG_BUFFER_PRINT(buffvar, txt)
#define TYM_DBG_BUFFER_PRINT_ENCLOSE(buffvar, txt, txtend)
#endif // TYM_DEBUG

#endif /* __TYM_BUFFER_H__ */
