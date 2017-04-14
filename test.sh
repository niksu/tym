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

for FILE in $(ls tests/*.test)
do
  CMD="./tym -i ${FILE} --test_parsing 2>&1"
  echo "Running \"${CMD}\""
  diff ${QUIET} <(eval ${CMD}) ${FILE}.expected
done
