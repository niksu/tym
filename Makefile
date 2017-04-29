# Makefile for TYM Datalog
# Nik Sultana, March 2017
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

CC?=gcc
CFLAGS=-Wall -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion -Wextra -g
TGT=tym
LIB=libtym.a
PARSER_OBJ=lexer.o parser.o
OBJ=ast.o formula.o statement.o symbols.o translate.o tym.o
HEADERS=ast.h formula.h statement.h symbols.h translate.h tym.h
STD=iso9899:1999
ADDITIONAL_CFLAGS?=

$(TGT) : $(LIB) $(HEADERS)
	$(CC) -std=$(STD) $(CFLAGS) $(ADDITIONAL_CFLAGS) -o $@ $(OBJ) $(PARSER_OBJ) -L. -ltym

$(LIB) : $(OBJ) $(HEADERS)
	ar crv $@ $(OBJ)

parser: $(HEADERS)
	bison -d -o parser.c parser.y
	flex lexer.l
	# We have to be more permissive with the C output of flex and bison :(
	$(CC) -c -std=$(STD) $(CFLAGS) $(ADDITIONAL_CFLAGS) -o lexer.o lexer.c
	$(CC) -c -std=$(STD) $(CFLAGS) $(ADDITIONAL_CFLAGS) -o parser.o parser.c

%.o: %.c $(HEADERS) parser
	$(CC) -c -std=$(STD) $(CFLAGS) -Werror $(ADDITIONAL_CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f $(TGT) $(LIB) *.o lexer.{c,h} parser.{c,h}
