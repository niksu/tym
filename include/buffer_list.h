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

#ifndef TYM_BUFFER_LIST_H
#define TYM_BUFFER_LIST_H

#include "buffer.h"
#include "buffer_internal.h"

void tym_reset_read_idx(struct TymBufferInfo * buf);
bool tym_more_to_read(struct TymBufferInfo * buf);
bool tym_progress_next_entry(struct TymBufferInfo * buf);

#endif /* TYM_BUFFER_LIST_H */
