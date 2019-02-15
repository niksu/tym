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
