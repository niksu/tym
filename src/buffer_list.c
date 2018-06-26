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

// FIXME this is an instance of "stateful reading" of the buffer, that can take place regardless of whether the buffer holds a single item or multiple items.
inline bool
tym_progress_next_entry(struct TymBufferInfo * buf)
{
  if ((buf->write_idx >= buf->buffer_size) ||
      (('\0' == buf->buffer[buf->write_idx + 1]) &&
       ('\0' == buf->buffer[buf->write_idx + 2]))) {
    return false;
  } else {
    while ((buf->write_idx < buf->buffer_size) ||
           ('\0' != buf->buffer[buf->write_idx])) {
      buf->write_idx += 1;
    }
    if (buf->write_idx < buf->buffer_size) {
       buf->write_idx += 1;
       return true;
    } else {
       return false;
    }
  }
}
