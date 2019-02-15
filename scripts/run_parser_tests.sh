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
