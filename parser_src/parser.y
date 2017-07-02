%{
/*
 * Bison spec.
 * Nik Sultana, March 2017.
 *
 * This is part of TYM Datalog (https://www.github.com/niksu/tym)
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
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

int yyerror(struct program_t ** program, yyscan_t scanner, const char * error_message);
struct program_t * parse(const char * string);

%}

%pure-parser

%lex-param   { yyscan_t scanner }
%parse-param { struct program_t ** program }
%parse-param { yyscan_t scanner }

%union {
  char * string;
  struct clause_t * clause;
  struct atom_t * atom;
  struct term_t * term;
  struct terms_t * terms;
  struct atoms_t * atoms;
  struct clauses_t * clauses;
  struct program_t * program;
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
         struct term_t * t = mk_term(CONST, identifier);
         $$ = t; }
     | TK_VAR
       { char * identifier = $1;
         struct term_t * t = mk_term(VAR, identifier);
         $$ = t; }
     | TK_STRING
       { char * identifier = $1;
         struct term_t * t = mk_term(STR, identifier);
         $$ = t; }

terms : term TK_R_RB
        { struct terms_t * ts = mk_term_cell($1, NULL);
          $$ = ts; }
      | term TK_COMMA terms
        { struct terms_t * ts = mk_term_cell($1, $3);
          $$ = ts; }
      | TK_R_RB
        { $$ = NULL; }

atom : TK_CONST TK_L_RB terms
       { char * predicate = $1;
         struct terms_t * ts = $3;
         struct atom_t * atom = mk_atom(predicate, len_Terms_cell(ts), ts);
         $$ = atom; }

atoms : atom
        { struct atoms_t * ats = mk_atom_cell($1, NULL);
          $$ = ats; }
      | atom TK_COMMA atoms
        { struct atoms_t * ats = mk_atom_cell($1, $3);
          $$ = ats; }

clause : atom TK_PERIOD
         { struct clause_t * cl = mk_clause($1, 0, NULL);
           $$ = cl; }
       | atom TK_IF atoms TK_PERIOD
         { struct atoms_t * ats = $3;
           struct clause_t * cl = mk_clause($1, len_Atoms_cell(ats), ats);
           $$ = cl; }

clauses : clause
          { struct clauses_t * cls = mk_clause_cell($1, NULL);
            $$ = cls; }
        | clause clauses
          { struct clauses_t * cls = mk_clause_cell($1, $2);
            $$ = cls; }

program : clauses
           { struct clauses_t * cls = $1;
             struct program_t * p = mk_program(len_Clauses_cell(cls), cls);
             *program = p;
           }
        |
           { struct program_t * p = mk_program(0, NULL);
             *program = p;
           }

%%

int yyerror(struct program_t ** program, yyscan_t scanner, const char * error_message) {
  ERR("parse error: %s\n", error_message);
  return 0;
}

struct program_t *
parse(const char * string)
{
  struct program_t * parsed = NULL;
  yyscan_t scanner;
  YY_BUFFER_STATE state;
  if (yylex_init(&scanner)) {
    ERR("yylex_init encountered a problem.");
    return NULL;
  }

  state = yy_scan_string(string, scanner);
  if (yyparse(&parsed, scanner)) {
    ERR("yyparse encountered a problem.");
    return NULL;
  }

  yy_delete_buffer(state, scanner);
  yylex_destroy(scanner);
  return parsed;
}
