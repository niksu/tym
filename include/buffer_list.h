/*
 * Buffer management for lists.
 * Nik Sultana, June 2018.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef TYM_BUFFER_LIST_H
#define TYM_BUFFER_LIST_H

#include "buffer.h"
#include "buffer_internal.h"

void tym_reset_idx(struct TymBufferInfo * buf);
void tym_done_last_entry(struct TymBufferInfo * buf);
bool tym_progress_next_entry(struct TymBufferInfo * buf);

#endif /* TYM_BUFFER_LIST_H */
