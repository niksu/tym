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


This file: Useful definitions of broad scope.
*/

#include "stdlib.h"
#include "string.h"

#include "util.h"

size_t TYM_BUF_SIZE = 3000;

char *
strcpy_prefixed(const char * prefix, const char * str)
{
  char * result = malloc(strlen(prefix) + strlen(str) + 1);
  memcpy(result, prefix, strlen(prefix));
  memcpy(result + strlen(prefix), str, strlen(str));
  result[strlen(prefix) + strlen(str)] = '\0';
  return result;
}

void
free_const(const void * m)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  free((void *)m);
#pragma GCC diagnostic pop
}
