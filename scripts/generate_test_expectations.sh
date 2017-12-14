#!/bin/bash
#
# TYM Datalog.
# Nik Sultana, March 2017.
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

set -e

[ -z "${TYMDIR}" ] && TYMDIR=.

for FILE in $(ls ${TYMDIR}/parser_tests/*.test)
do
  CMD="${TYMDIR}/out/tym -i ${FILE} -f test_parsing > ${FILE}.expected"
  echo "Running \"${CMD}\""
  eval ${CMD}
done
