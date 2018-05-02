#!/bin/bash
#
# Wrapper for cc driver to link Tym-generated C code.
# Nik Sultana, April 2018
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

OUTFILE=$1
gcc -Iinclude/ -c ${OUTFILE}.c
# FIXME hardcoded path to tym_runtime.o
gcc -Lout -ltym -o ${OUTFILE} out/tym_runtime.o ${OUTFILE}.o
