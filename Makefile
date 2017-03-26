# Makefile for TYM Datalog
# Nik Sultana, March 2017
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

CC?=gcc
CFLAGS=-std=c99 -Wall -pedantic
TGT=tym
OBJ=ast.o formula.o lexer.o parser.o symbols.o tym.o
HEADERS=ast.h formula.h lexer.h parser.h symbols.h tym.h
ADDITIONAL_CFLAGS?=

$(TGT) : $(OBJ) $(HEADERS)
	$(CC) -o $@ $(CFLAGS) $(ADDITIONAL_CFLAGS) $(OBJ)

lexer.c : lexer.l
	flex $<

lexer.h : lexer.l
	flex $<

parser.h : parser.y
	bison -d -o parser.c $<

parser.c : parser.y
	bison -d -o parser.c $<

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $(CFLAGS) $(ADDITIONAL_CFLAGS) $<

.PHONY: clean

clean:
	rm -f $(TGT) *.o lexer.{c,h} parser.{c,h}
