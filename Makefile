# Makefile for TYM Datalog
# Nik Sultana, March 2017
# https://github.com/niksu/tym
#
# License: LGPL version 3 (for licensing terms see the file called LICENSE)

CC?=gcc
CFLAGS=-std=c99 -Wall -pedantic
TGT=tym
OBJ=tym.o ast.o lexer.o parser.o
HEADERS=ast.h lexer.h parser.h tym.h

$(TGT) : $(OBJ) $(HEADERS)
	$(CC) -o $@ $(CFLAGS) $(OBJ)

lexer.c : lexer.l
	flex $<

lexer.h : lexer.l
	flex $<

parser.h : parser.y
	bison -d -o parser.c $<

parser.c : parser.y
	bison -d -o parser.c $<

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $(CFLAGS) $<

.PHONY: clean

clean:
	rm -f $(TGT) *.o lexer.{c,h} parser.{c,h}
