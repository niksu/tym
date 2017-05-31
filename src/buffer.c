/*
 * Buffer management.
 * Nik Sultana, May 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <stdlib.h>
#include <string.h>

#include "buffer.h"

struct buffer_info *
mk_buffer(const size_t buffer_size)
{
  char * b = malloc(sizeof(char) * buffer_size);
  assert(NULL != b);
  struct buffer_info * buf = malloc(sizeof(struct buffer_info));
  *buf = (struct buffer_info){.buffer = b, .idx = 0, .buffer_size = buffer_size};
  return buf;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
void
free_buffer(struct buffer_info * buf)
{
  free((void *)buf->buffer);
  free((void *)buf);
}
#pragma GCC diagnostic pop

bool
have_space(struct buffer_info * buf, size_t n)
{
  if (n < buf->buffer_size - buf->idx) {
    return true;
  } else {
    return false;
  }
}

inline void
unsafe_buffer_char(struct buffer_info * buf, char c)
{
  buf->buffer[buf->idx] = c;
  buf->idx += 1;
}

inline void
safe_buffer_replace_last(struct buffer_info * buf, char c)
{
  assert(NULL != buf);
  assert(buf->idx > 0);
  buf->buffer[buf->idx - 1] = c;
}

inline void
unsafe_buffer_str(struct buffer_info * buf, char * s)
{
  strcpy(buf->buffer + buf->idx, s);
  // NOTE the updated idx doesn't include the terminating null character, which
  //      strcpy preserves.
  buf->idx += strlen(s);
}

inline void
unsafe_dec_idx(struct buffer_info * buf, size_t n)
{
  buf->idx -= n;
}

struct buffer_write_result *
buf_strcpy(struct buffer_info * dst, const char * src)
{
  size_t l = strlen(src) + 1; // NOTE we include \0 in the size of the string.
  if (have_space(dst, l)) {
    strcpy(dst->buffer + dst->idx, src);
    dst->idx += l;
    return mkval_buffer_write_result(l);
  } else {
    return mkerrval_buffer_write_result(BUFF_ERR_OVERFLOW);
  }
}


MAYBE_ERROR__IS_OK_DEF(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__VAL_OF_DEF(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__ERRVAL_OF_DEF(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__MKVAL_DEF(buffer_write_result, size_t, enum buffer_errors)
MAYBE_ERROR__MKERRVAL_DEF(buffer_write_result, size_t, enum buffer_errors)
