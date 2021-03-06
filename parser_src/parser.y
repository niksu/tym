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


This file: Bison spec.
*/

#include <stdio.h>

#include "ast.h"
#include "parser.h"
#include "lexer.h"
#include "util.h"

#define YYERROR_VERBOSE 1

#define TERM_BUFFER 300
#define ATOM_BUFFER 300
#define CLAUSE_BUFFER 300

// FIXME could add checks to ensure that syntactic structures fit within a
//       certain size, e.g., by using constraints such as those below:
//#define MAX_NO_ATOM_ARGS 30
//#define MAX_CLAUSE_BODY_SIZE 30

int yyerror(struct TymProgram ** program, yyscan_t scanner, const char * error_message);
struct TymProgram * parse(const char * string);

%}

%pure-parser

%lex-param   { yyscan_t scanner }
%parse-param { struct TymProgram ** program }
%parse-param { yyscan_t scanner }

%union {
  char * string;
  struct TymClause * clause;
  struct TymAtom * atom;
  struct TymTerm * term;
  struct TymTerms * terms;
  struct TymAtoms * atoms;
  struct TymClauses * clauses;
  struct TymProgram * program;
}

%token TK_L_RB
%token TK_R_RB
%token <string> TK_VAR
%token <string> TK_CONST
%token <string> TK_STRING
%token TK_COMMA
%token TK_IF
%token TK_PERIOD

%right TK_COMMA

%type <terms> terms
%type <term> term
%type <atoms> atoms
%type <atom> atom
%type <clause> clause
%type <clauses> clauses
%type <program> program
%start program
%%

term : TK_CONST
       { char * identifier = $1;
         struct TymTerm * t = tym_mk_term(TYM_CONST,
           tym_encode_str(strcpy_prefixed(TYM_CONST_PREFIX, identifier)));
         free(identifier);
         $$ = t; }
     | TK_VAR
       { char * identifier = $1;
         struct TymTerm * t = tym_mk_term(TYM_VAR,
           tym_encode_str(identifier));
         $$ = t; }
     | TK_STRING
       { char * identifier = $1;
         struct TymTerm * t = tym_mk_term(TYM_STR,
           tym_encode_str(identifier));
         $$ = t; }

terms : term TK_R_RB
        { struct TymTerms * ts = tym_mk_term_cell($1, NULL);
          $$ = ts; }
      | term TK_COMMA terms
        { struct TymTerms * ts = tym_mk_term_cell($1, $3);
          $$ = ts; }
      | TK_R_RB
        { $$ = NULL; }

atom : TK_CONST TK_L_RB terms
       { char * predicate = $1;
         struct TymTerms * ts = $3;
         struct TymAtom * atom = tym_mk_atom(tym_encode_str(strcpy_prefixed(TYM_PREDICATE_PREFIX, predicate)),
           tym_len_TymTerms_cell(ts), ts);
         free(predicate);
         $$ = atom; }

atoms : atom
        { struct TymAtoms * ats = tym_mk_atom_cell($1, NULL);
          $$ = ats; }
      | atom TK_COMMA atoms
        { struct TymAtoms * ats = tym_mk_atom_cell($1, $3);
          $$ = ats; }

clause : atom TK_PERIOD
         { struct TymClause * cl = tym_mk_clause($1, 0, NULL);
           $$ = cl; }
       | atom TK_IF atoms TK_PERIOD
         { struct TymAtoms * ats = $3;
           struct TymClause * cl = tym_mk_clause($1, tym_len_TymAtoms_cell(ats), ats);
           $$ = cl; }

clauses : clause
          { struct TymClauses * cls = tym_mk_clause_cell($1, NULL);
            $$ = cls; }
        | clause clauses
          { struct TymClauses * cls = tym_mk_clause_cell($1, $2);
            $$ = cls; }

program : clauses
           { struct TymClauses * cls = $1;
             struct TymProgram * p = tym_mk_program(tym_len_TymClauses_cell(cls), cls);
             *program = p;
           }
        |
           { struct TymProgram * p = tym_mk_program(0, NULL);
             *program = p;
           }

%%

int yyerror(struct TymProgram ** program, yyscan_t scanner, const char * error_message) {
  TYM_ERR("parse error: %s\n", error_message);
  return 0;
}

struct TymProgram *
parse(const char * string)
{
  struct TymProgram * parsed = NULL;
  yyscan_t scanner;
  YY_BUFFER_STATE state;
  if (yylex_init(&scanner)) {
    TYM_ERR("yylex_init encountered a problem.");
    return NULL;
  }

  state = yy_scan_string(string, scanner);
  if (yyparse(&parsed, scanner)) {
    TYM_ERR("yyparse encountered a problem.");
    return NULL;
  }

  yy_delete_buffer(state, scanner);
  yylex_destroy(scanner);
  return parsed;
}
