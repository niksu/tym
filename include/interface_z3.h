/*
Copyright Nik Sultana, 2019

This file is part of TYM Datalog. (https://www.github.com/niksu/tym)

TYM Datalog is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TYM Datalog is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details, a copy of which
is included in the file called LICENSE distributed with TYM Datalog.

You should have received a copy of the GNU Lesser General Public License
along with TYM Datalog.  If not, see <https://www.gnu.org/licenses/>.


This file: Translation from formulas to Z3's API.
*/

#ifndef TYM_INTERFACE_Z3_H
#define TYM_INTERFACE_Z3_H

#include <assert.h>
#include <z3.h>

#include "ast.h"
#include "formula.h"
#include "string_idx.h"
#include "support.h"

void tym_z3_begin(struct TymParams *);
void tym_z3_end(void);
enum TymSatisfiable tym_z3_satisfied(void);
void tym_z3_check(void);
void tym_z3_assert_smtlib2(const char * str);

void tym_z3_get_model(struct TymMdlValuations *);
void tym_z3_print_model(void);

#endif // TYM_INTERFACE_Z3_H
