# Makefile for TYM Datalog
# Nik Sultana, March 2017
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

CC?=gcc
REDUCED_CFLAGS=-Wall -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion -Wextra
CFLAGS=-Wall -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion -Werror -Wextra
TGT=tym
OBJ=tym.o ast.o
PARSER_OBJ=lexer.o parser.o
#HEADERS=ast.h lexer.h parser.h tym.h
HEADERS=ast.h tym.h
STD=iso9899:1999
ADDITIONAL_CFLAGS?=

$(TGT) : $(OBJ) $(HEADERS)
	$(CC) -std=$(STD) $(CFLAGS) $(ADDITIONAL_CFLAGS) -o $@ $(OBJ) $(PARSER_OBJ)

parser: $(HEADERS)
	bison -d -o parser.c parser.y
	flex lexer.l
	# We have to be more permissive with the C output of flex and bison :(
	$(CC) -c -std=$(STD) $(REDUCED_CFLAGS) $(ADDITIONAL_CFLAGS) -o lexer.o lexer.c
	$(CC) -c -std=$(STD) $(REDUCED_CFLAGS) $(ADDITIONAL_CFLAGS) -o parser.o parser.c

%.o: %.c $(HEADERS) parser
	$(CC) -c -std=$(STD) $(CFLAGS) $(ADDITIONAL_CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f $(TGT) *.o lexer.{c,h} parser.{c,h}
