#!/bin/bash
#
# Wrapper for cc driver to compile and link Tym-generated C code.
# Nik Sultana, April 2018
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

OUTFILE=$1
gcc ${CFLAGS} -Iinclude/ -Iout -c ${OUTFILE}.c
# FIXME hardcoded path to tym_runtime.o
gcc ${CFLAGS} -Lout -ltym -o ${OUTFILE} out/tym_runtime.o ${OUTFILE}.o
