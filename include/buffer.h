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

#include "util.h"

struct TymBufferInfo {
  char * const buffer;
  size_t idx;
  const size_t buffer_size;
};

struct TymBufferInfo * tym_mk_buffer(const size_t buffer_size);
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

struct TymBufferWriteResult * tym_buf_strcpy(struct TymBufferInfo * dst, const char * src);

void tym_buff_error_msg(void * x);
TYM_ERROR_CHECK_DECL(TymBufferWriteResult, size_t, enum TymBufferErrors, buff_error_msg)

#endif /* __TYM_BUFFER_H__ */
