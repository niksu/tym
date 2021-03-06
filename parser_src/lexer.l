%{
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


This file: Lex spec.
 */

#include <stdio.h>

#include "ast.h"
#include "parser.h"
#include "util.h"
%}

%option outfile="lexer.c" header-file="lexer.h"
%option warn nodefault 8bit bison-bridge reentrant
%option never-interactive nounistd nounput nomain noyywrap noinput

PERCENT "%"
L_ROUND_BRACKET "("
COMMA ","
R_ROUND_BRACKET ")"
EQUALS "="
IF ":-"
PERIOD "."
TILDE "~"
QUESTION "?"
QUOTE "\""
WS [ \t\n\r]*
DECIMAL [0-9]
UPPERALPHA [A-Z]
LOWERALPHA [a-z]
NAMEBODY (_|{DECIMAL}|{UPPERALPHA}|{LOWERALPHA})
VAR {UPPERALPHA}+{NAMEBODY}*
/* NOTE definition here is more conservative than at
   http://www.ccs.neu.edu/home/ramsdell/tools/datalog/datalog.html*/
CONST {LOWERALPHA}+{NAMEBODY}*
STRING {QUOTE}.*{QUOTE}

%%

{WS} { }
{PERCENT}[^\n]* { }

{L_ROUND_BRACKET} {return TK_L_RB;}
{R_ROUND_BRACKET} {return TK_R_RB;}
{VAR} {yylval->string = strdup(yytext); return TK_VAR;}
{CONST} {yylval->string = strdup(yytext); return TK_CONST;}
{STRING} {yylval->string = strdup(yytext); return TK_STRING;}
{COMMA} {return TK_COMMA;}
{IF} {return TK_IF;}
{PERIOD} {return TK_PERIOD;}
. {TYM_ERR("Unrecognised token: %s\n", yytext);}

%%

/*
int yyerror(const char *error_message) {
  TYM_ERR("Tokenisation error: %s\n", error_message);
}
*/
