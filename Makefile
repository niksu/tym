# Makefile for TYM Datalog
# Nik Sultana, March 2017
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

CC?=gcc
CFLAGS+=-Wall -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion -Wextra -g
TGT=tym
LIB=libtym.a
PARSER_OBJ=lexer.o parser.o
OBJ=ast.o formula.o statement.o symbols.o translate.o
OBJ_OF_TGT=main.o
HEADER_FILES=ast.h formula.h statement.h symbols.h translate.h util.h
HEADER_DIR=include
HEADERS=$(addprefix $(HEADER_DIR)/, $(HEADER_FILES))
STD=iso9899:1999

$(TGT) : $(LIB) $(OBJ_OF_TGT) $(HEADERS)
	$(CC) -std=$(STD) $(CFLAGS) -o $@ $(OBJ) $(OBJ_OF_TGT) $(PARSER_OBJ) -L. -ltym -I $(HEADER_DIR)

$(LIB) : $(OBJ) $(HEADERS)
	ar crv $@ $(OBJ)

parser: $(HEADERS) src_parser/parser.y src_parser/lexer.l
	bison -d -o parser.c src_parser/parser.y
	flex src_parser/lexer.l
	# We have to be more permissive with the C output of flex and bison :(
	$(CC) -c -std=$(STD) $(CFLAGS) -I $(HEADER_DIR) -o lexer.o lexer.c
	$(CC) -c -std=$(STD) $(CFLAGS) -I $(HEADER_DIR) -o parser.o parser.c

%.o: src/%.c $(HEADERS) parser
	$(CC) -c -std=$(STD) $(CFLAGS) -Werror -I $(HEADER_DIR) -I . -o $@ $<

.PHONY: clean test

test:
	@TYMDIR=`pwd` ./scripts/run_tests.sh

clean:
	rm -f $(TGT) $(LIB) *.o lexer.{c,h} parser.{c,h}
