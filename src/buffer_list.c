/*
 * Buffer management for lists.
 * Nik Sultana, June 2018.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include "buffer_list.h"

inline void
tym_done_last_entry(struct TymBufferInfo * buf)
{
  tym_unsafe_buffer_str(buf, "");
}

inline bool
tym_progress_next_entry(struct TymBufferInfo * buf)
{
  if ((buf->idx >= buf->buffer_size) ||
      (('\0' == buf->buffer[buf->idx + 1]) &&
       ('\0' == buf->buffer[buf->idx + 2]))) {
    return false;
  } else {
    while ((buf->idx < buf->buffer_size) ||
           ('\0' != buf->buffer[buf->idx])) {
      buf->idx += 1;
    }
    if (buf->idx < buf->buffer_size) {
       buf->idx += 1;
       return true;
    } else {
       return false;
    }
  }
}
