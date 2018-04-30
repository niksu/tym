#!/bin/bash
#
# Wrapper for cc driver to compile Tym-generated C code.
# Nik Sultana, April 2018
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

OUTFILE=$1
gcc -g -Iinclude/ -Iout/ -Lout -ltym -o ${OUTFILE} ${OUTFILE}.c
