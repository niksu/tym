#!/bin/bash
#
# TYM Datalog.
# Nik Sultana, March 2017.
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

set -e
for FILE in $(ls tests/*.test)
do
  CMD="./tym -i ${FILE} --test_parsing"
  echo "Running \"${CMD}\""
  diff -q <(eval ${CMD}) ${FILE}.expected
done
