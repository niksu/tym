# Makefile for TYM Datalog
# Nik Sultana, March 2017
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

CC?=gcc
CFLAGS+=-Wall -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align \
				-Wstrict-prototypes -Wmissing-prototypes -Wconversion -Wextra -g \
#				-fprofile-arcs -ftest-coverage -O0
TGT=tym
LIB=libtym.a
OUT_DIR=out
PARSER_OBJ=$(OUT_DIR)/lexer.o $(OUT_DIR)/parser.o
OBJ_FILES=ast.o formula.o statement.o symbols.o translate.o
OBJ=$(addprefix $(OUT_DIR)/, $(OBJ_FILES))
OBJ_OF_TGT=$(OUT_DIR)/main.o
HEADER_FILES=ast.h formula.h statement.h symbols.h translate.h util.h
HEADER_DIR=include
HEADERS=$(addprefix $(HEADER_DIR)/, $(HEADER_FILES))
STD=iso9899:1999

$(TGT) : $(LIB) $(OBJ_OF_TGT) $(HEADERS)
	mkdir -p $(OUT_DIR)
	$(CC) -std=$(STD) $(CFLAGS) -o $(OUT_DIR)/$@ $(OBJ) $(OBJ_OF_TGT) $(PARSER_OBJ) -L $(OUT_DIR) -ltym -I $(HEADER_DIR)

$(LIB) : $(OBJ) $(HEADERS)
	mkdir -p $(OUT_DIR)
	ar crv $(OUT_DIR)/$@ $(OBJ)

parser: $(HEADERS) src_parser/parser.y src_parser/lexer.l
	mkdir -p $(OUT_DIR)
	bison -d -o parser.c src_parser/parser.y
	flex src_parser/lexer.l
	mv lexer.{c,h} $(OUT_DIR)
	mv parser.{c,h} $(OUT_DIR)
	# We have to be more permissive with the C output of flex and bison :(
	$(CC) -c -std=$(STD) $(CFLAGS) -I $(HEADER_DIR) -o $(OUT_DIR)/lexer.o $(OUT_DIR)/lexer.c
	$(CC) -c -std=$(STD) $(CFLAGS) -I $(HEADER_DIR) -o $(OUT_DIR)/parser.o $(OUT_DIR)/parser.c

out/%.o: src/%.c $(HEADERS) parser
	mkdir -p $(OUT_DIR)
	$(CC) -c -std=$(STD) $(CFLAGS) -Werror -I $(HEADER_DIR) -I $(OUT_DIR) -o $@ $<

.PHONY: clean test

test_regression:
	@TYMDIR=`pwd` ./scripts/run_tests.sh

clean:
	rm -f $(OUT_DIR)/$(TGT) $(OUT_DIR)/$(LIB) $(OUT_DIR)/*.o $(OUT_DIR)/lexer.{c,h} $(OUT_DIR)/parser.{c,h}
