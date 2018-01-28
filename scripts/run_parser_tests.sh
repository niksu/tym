#!/bin/bash
#
# TYM Datalog.
# Nik Sultana, March 2017.
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

QUIET=""
if [ "${SUMMARY}" == "1" ]
then
  QUIET="-q"
fi

if [ "${FAIL_EARLY}" == "1" ]
then
  set -e
fi

[ -z "${TYMDIR}" ] && TYMDIR=.
[ -z "${PREFIX}" ] && PREFIX="valgrind --leak-check=full"
if [ -n "${TYM_Z3_PATH}" ]
then
  DLP="DYLD_LIBRARY_PATH=${TYM_Z3_PATH}/bin/"
else
  DLP=
fi

for FILE in $(ls ${TYMDIR}/parser_tests/*.test)
do
  if [ "${MEM_CHECK}" == "1" ]
  then
    CMD="${DLP} ${PREFIX} ${TYMDIR}/out/tym -i ${FILE} -f test_parsing 2>&1"
    echo "Running \"${CMD}\""
    eval ${CMD} | grep "ERROR SUMMARY"
  else
    CMD="${DLP} ${TYMDIR}/out/tym -i ${FILE} -f test_parsing 2>&1"
    echo "Running \"${CMD}\""
    diff ${QUIET} <(eval ${CMD}) ${FILE}.expected
  fi
done
