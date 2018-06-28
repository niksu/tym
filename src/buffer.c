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
#include "buffer_internal.h"

const char *
tym_buffer_contents(struct TymBufferInfo * buf)
{
  return buf->buffer + buf->read_idx;
}

size_t
tym_buffer_len(struct TymBufferInfo * buf)
{
  return buf->write_idx;
}

size_t
tym_buffer_size(struct TymBufferInfo * buf)
{
  return buf->buffer_size;
}

struct TymBufferInfo *
tym_mk_buffer(const size_t buffer_size)
{
  char * b = malloc(sizeof(char) * buffer_size);
  assert(NULL != b);
  memset(b, '\0', buffer_size);
  struct TymBufferInfo * buf = malloc(sizeof *buf);
  *buf = (struct TymBufferInfo){.buffer = b, .write_idx = 0, .buffer_size = buffer_size};
  return buf;
}

void
tym_reset_buffer(struct TymBufferInfo * buf)
{
  buf->write_idx = 0;
  buf->read_idx = 0;
  buf->buffer[0] = '\0';
}

void
tym_free_buffer(struct TymBufferInfo * buf)
{
  free((void *)buf->buffer);
  free((void *)buf);
}

bool
tym_have_space(struct TymBufferInfo * buf, size_t n)
{
  assert(buf->buffer_size > 0);
  assert(buf->buffer_size > buf->write_idx);
  return (n < buf->buffer_size - buf->write_idx);
}

inline void
tym_unsafe_buffer_char(struct TymBufferInfo * buf, char c)
{
  buf->buffer[buf->write_idx] = c;
  buf->write_idx += 1;
}

inline void
tym_safe_buffer_replace_last(struct TymBufferInfo * buf, char c)
{
  assert(NULL != buf);
  assert(buf->write_idx > 0);
  // buf->write_idx points at the next address to write, and here we change a location
  // we've written to, so we go negative.
  buf->buffer[buf->write_idx - 1] = c;
}

inline void
tym_unsafe_buffer_str(struct TymBufferInfo * buf, char * s)
{
  strcpy(buf->buffer + buf->write_idx, s);
  // NOTE the updated write_idx doesn't include the terminating null character, which
  //      strcpy preserves.
  buf->write_idx += strlen(s);
}

inline void
tym_unsafe_dec_idx(struct TymBufferInfo * buf, size_t n)
{
  // FIXME could be made safe by ensuring that "write_idx >= n"
  buf->write_idx -= n;
}

struct TYM_LIFTED_TYPE_NAME(TymBufferWriteResult) *
tym_buf_strcpy(struct TymBufferInfo * dst, const char * src)
{
  assert(NULL != src);

  size_t l = strlen(src) + 1; // NOTE we include \0 in the size of the string.
  if (tym_have_space(dst, l)) {
    strcpy(dst->buffer + dst->write_idx, src);
    dst->write_idx += l;
    return tym_mkval_TymBufferWriteResult(l);
  } else {
    return tym_mkerrval_TymBufferWriteResult(BUFF_ERR_OVERFLOW);
  }
}

TYM_MAYBE_ERROR__IS_OK_DEFN(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__VAL_OF_DEFN(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__ERRVAL_OF_DEFN(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__MKVAL_DEFN(TymBufferWriteResult, size_t, enum TymBufferErrors)
TYM_MAYBE_ERROR__MKERRVAL_DEFN(TymBufferWriteResult, size_t, enum TymBufferErrors)

//Error handler -- if something's wrong with the buffer then explain what's
//wrong, and terminate.
void
tym_buff_error_msg(void * x)
{
  struct TymBufferInfo * buf = (struct TymBufferInfo *)x;
  if (buf->write_idx >= buf->buffer_size) {
    buf->write_idx = buf->buffer_size - 1;
  }
  buf->buffer[buf->write_idx] = '\0';
  fprintf(stderr, "Buffer error (write_idx=%zu, size=%zu, remaining=%zu)\n|%s|\n",
      buf->write_idx, buf->buffer_size, buf->buffer_size - buf->write_idx, buf->buffer);
  assert(false);
}

TYM_ERROR_CHECK_DEFN(TymBufferWriteResult, size_t, enum TymBufferErrors, tym_buff_error_msg)
