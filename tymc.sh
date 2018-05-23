#!/bin/bash
#
# Wrapper for cc driver to compile and link Tym-generated C code.
# Nik Sultana, April 2018
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

OUTFILE=$1

if [ -z "${TYM}" ]
then
  # if $TYM isn't defined, then we assume to be in the Tym directory
  TYM_DEV=.
  TYM_LIB=./out/
fi

OUTFILE=$1
gcc ${CFLAGS} -I${TYM_DEV}/include/ -I${TYM_DEV}/out -c ${OUTFILE}.c
gcc ${CFLAGS} -L${TYM_LIB} -ltym -o ${OUTFILE} ${TYM_LIB}/tym_runtime.o ${OUTFILE}.o
