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

CC?=gcc
CFLAGS+=-Wall -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align \
				-Wstrict-prototypes -Wmissing-prototypes -Wconversion -Wextra -g \
#				-fprofile-arcs -ftest-coverage -O0
TGT=tym
LIB=libtym.a
OUT_DIR=out
PARSER_OBJ=$(OUT_DIR)/lexer.o $(OUT_DIR)/parser.o
OBJ_FILES=ast.o buffer.o buffer_list.o formula.o hash.o hashtable.o interface_c.o output_c.o statement.o string_idx.o support.o symbols.o translate.o util.o
OBJ=$(addprefix $(OUT_DIR)/, $(OBJ_FILES))
OBJ_OF_TGT=$(OUT_DIR)/main.o
HEADER_FILES=ast.h buffer.h buffer_list.h formula.h hash.h hashtable.h interface_c.h output_c.h lifted.h statement.h string_idx.h support.h symbols.h translate.h util.h
HEADER_DIR=include
HEADERS=$(addprefix $(HEADER_DIR)/, $(HEADER_FILES))
STD=iso9899:1999

ifdef TYM_Z3_PATH
Z3_LINK?=-L $(TYM_Z3_PATH)/bin -lz3
Z3_INC?=-I $(TYM_Z3_PATH)/include
OBJ_FILES+=interface_z3.o
HEADER_FILES+=interface_z3.h
CFLAGS+=-DTYM_INTERFACE_Z3
endif

$(TGT) : $(LIB) $(OBJ_OF_TGT) $(HEADERS)
	mkdir -p $(OUT_DIR)
	$(CC) -std=$(STD) $(CFLAGS) -o $(OUT_DIR)/$@ $(OBJ) $(OBJ_OF_TGT) $(PARSER_OBJ) -L $(OUT_DIR) -ltym -I $(HEADER_DIR) $(Z3_LINK)

$(LIB) : $(OBJ) $(HEADERS)
	mkdir -p $(OUT_DIR)
	ar crv $(OUT_DIR)/$@ $(OBJ) $(PARSER_OBJ)

parser: $(HEADERS) parser_src/parser.y parser_src/lexer.l
	mkdir -p $(OUT_DIR)
	bison -d -o parser.c parser_src/parser.y
	flex parser_src/lexer.l
	mv lexer.{c,h} $(OUT_DIR)
	mv parser.{c,h} $(OUT_DIR)
	# We have to be more permissive with the C output of flex and bison, thus "-Werror"
	# is excluded when compiling code they produced.
	$(CC) -c -std=$(STD) $(CFLAGS) -I $(HEADER_DIR) -o $(OUT_DIR)/lexer.o $(OUT_DIR)/lexer.c
	$(CC) -c -std=$(STD) $(CFLAGS) -I $(HEADER_DIR) -o $(OUT_DIR)/parser.o $(OUT_DIR)/parser.c

$(OUT_DIR)/%.o: src/%.c $(HEADERS) parser
	mkdir -p $(OUT_DIR)
	$(CC) -c -std=$(STD) $(CFLAGS) -Werror -I $(HEADER_DIR) -I $(OUT_DIR) $(Z3_INC) -o $@ $<

.PHONY: clean test_modules test_regression

test_modules:
	make clean
	CFLAGS="$(CFLAGS) -DTYM_TESTING" make $(TGT)
	./$(OUT_DIR)/$(TGT)

test_regression:
	@TYM_Z3_PATH="$(TYM_Z3_PATH)" TYMDIR=`pwd` ./scripts/run_parser_tests.sh

$(OUT_DIR)/tym_runtime.o : $(LIB) $(HEADERS)
	mkdir -p $(OUT_DIR)
	$(CC) -DTYM_PRECODED -std=$(STD) $(CFLAGS) -c -o $(OUT_DIR)/tym_runtime.o src/main.c -I $(OUT_DIR) -I $(HEADER_DIR)

clean:
	rm -f $(OUT_DIR)/$(TGT) $(OUT_DIR)/$(LIB) $(OUT_DIR)/*.o $(OUT_DIR)/lexer.{c,h} $(OUT_DIR)/parser.{c,h}
