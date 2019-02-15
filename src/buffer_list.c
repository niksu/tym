/*
Copyright Nik Sultana, 2019

This file is part of TYM Datalog. (https://www.github.com/niksu/tym)

TYM Datalog is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TYM Datalog is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details, a copy of which
is included in the file called LICENSE distributed with TYM Datalog.

You should have received a copy of the GNU Lesser General Public License
along with TYM Datalog.  If not, see <https://www.gnu.org/licenses/>.


This file: Buffer management for lists.
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
