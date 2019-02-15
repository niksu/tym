#!/bin/bash
#
# Copyright Nik Sultana, 2019
#
# This file is part of TYM Datalog. (https://www.github.com/niksu/tym)
#
# TYM Datalog is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# TYM Datalog is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details, a copy of which
# is included in the file called LICENSE distributed with TYM Datalog.
#
# You should have received a copy of the GNU Lesser General Public License
# along with TYM Datalog.  If not, see <https://www.gnu.org/licenses/>.
#
#
# This file: Wrapper for cc driver to compile and link Tym-generated C code.

OUTFILE=$1
gcc ${CFLAGS} -c ${OUTFILE}.c
gcc ${CFLAGS} -ltym -o ${OUTFILE} ${TYM}/tym_runtime.o ${OUTFILE}.o
