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

struct buffer_info {
  char * const buffer;
  size_t idx;
  const size_t buffer_size;
};

struct buffer_info * mk_buffer(const size_t buffer_size);
void free_buffer(struct buffer_info * buf);
bool have_space(struct buffer_info * buf, size_t n);
void unsafe_buffer_char(struct buffer_info * buf, char c);
void unsafe_buffer_str(struct buffer_info * buf, char * s);
void unsafe_dec_idx(struct buffer_info * buf, size_t n);
void safe_buffer_replace_last(struct buffer_info * buf, char c);

struct buffer_write_result * buf_strcpy(struct buffer_info * dst, const char * src);

enum buffer_errors {NON_BUFF_ERROR, BUFF_ERR_OVERFLOW};

MAYBE_ERROR(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__IS_OK_DECL(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__VAL_OF_DECL(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__ERRVAL_OF_DECL(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__MKVAL_DECL(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__MKERRVAL_DECL(buffer_write_result, size_t, enum buffer_errors)

void buff_error_msg(void * x);
ERROR_CHECK_DECL(buffer_write_result, size_t, enum buffer_errors, buff_error_msg)

#endif /* __TYM_BUFFER_H__ */
