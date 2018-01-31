#!/bin/bash
#
# TYM Datalog.
# Nik Sultana, March 2017.
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

set -e

[ -z "${TYMDIR}" ] && TYMDIR=.
if [ -n "${TYM_Z3_PATH}" ]
then
  DLP="DYLD_LIBRARY_PATH=${TYM_Z3_PATH}/bin/"
else
  DLP=
fi

for FILE in $(ls ${TYMDIR}/parser_tests/*.test)
do
  CMD="${DLP} ${TYMDIR}/out/tym -i ${FILE} -f test_parsing > ${FILE}.expected"
  echo "Running \"${CMD}\""
  eval ${CMD}
done
