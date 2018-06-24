/*
 * Buffer management.
 * Nik Sultana, May 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#ifndef TYM_BUFFER_INTERNAL_H
#define TYM_BUFFER_INTERNAL_H

struct TymBufferInfo {
  char * buffer;
  size_t write_idx;
  size_t buffer_size;
};

#endif /* TYM_BUFFER_INTERNAL_H */
