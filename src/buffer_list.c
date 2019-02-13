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
tym_reset_read_idx(struct TymBufferInfo * buf)
{
  buf->read_idx = 0;
}

inline bool
tym_more_to_read(struct TymBufferInfo * buf)
{
  return (buf->read_idx < buf->write_idx - 1);
}

// FIXME this is an instance of "stateful reading" of the buffer, that can take place regardless of whether the buffer holds a single item or multiple items.
inline bool
tym_progress_next_entry(struct TymBufferInfo * buf)
{
  bool result = tym_more_to_read(buf);

  if (result) {
    while (tym_more_to_read(buf) ||
           ('\0' != buf->buffer[buf->read_idx])) {
      buf->read_idx += 1;
    }
    if (tym_more_to_read(buf)) {
       buf->read_idx += 1;
       result = tym_more_to_read(buf);
    } else {
       result = false;
    }
  }

  return result;
}
