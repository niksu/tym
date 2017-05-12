%{
/*
 * Lex spec.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
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
STRING {QUOTE}[^{QUOTE}]*{QUOTE}

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
. {ERR("Unrecognised token: %s\n", yytext);}

%%

/*
int yyerror(const char *error_message) {
  ERR("Tokenisation error: %s\n", error_message);
}
*/
