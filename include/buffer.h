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

struct TymBufferInfo * mk_buffer(const size_t buffer_size);
void free_buffer(struct TymBufferInfo * buf);
bool have_space(struct TymBufferInfo * buf, size_t n);
void unsafe_buffer_char(struct TymBufferInfo * buf, char c);
void unsafe_buffer_str(struct TymBufferInfo * buf, char * s);
void unsafe_dec_idx(struct TymBufferInfo * buf, size_t n);
void safe_buffer_replace_last(struct TymBufferInfo * buf, char c);

struct buffer_write_result * buf_strcpy(struct TymBufferInfo * dst, const char * src);

enum TymBufferErrors {NON_BUFF_ERROR, BUFF_ERR_OVERFLOW};

MAYBE_ERROR(buffer_write_result, size_t, enum TymBufferErrors)
MAYBE_ERROR__IS_OK_DECL(buffer_write_result, size_t, enum TymBufferErrors)
MAYBE_ERROR__VAL_OF_DECL(buffer_write_result, size_t, enum TymBufferErrors)
MAYBE_ERROR__ERRVAL_OF_DECL(buffer_write_result, size_t, enum TymBufferErrors)
MAYBE_ERROR__MKVAL_DECL(buffer_write_result, size_t, enum TymBufferErrors)
MAYBE_ERROR__MKERRVAL_DECL(buffer_write_result, size_t, enum TymBufferErrors)

void buff_error_msg(void * x);
ERROR_CHECK_DECL(buffer_write_result, size_t, enum TymBufferErrors, buff_error_msg)

#endif /* __TYM_BUFFER_H__ */
