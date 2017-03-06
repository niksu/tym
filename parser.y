%{
/*
 * TYM Datalog.
 * Nik Sultana, March 2017.
 *
 * License: LGPL version 3 (for licensing terms see the file called LICENSE)
 */

#include <stdio.h>
#include "ast.h"
#include "parser.h"
#include "lexer.h"

#define YYERROR_VERBOSE 1:w

#define TERM_BUFFER 300
#define ATOM_BUFFER 300
#define CLAUSE_BUFFER 300

int yyerror(struct clause_t ***clause, yyscan_t scanner, const char * error_message);

struct term_t * terms_buf[TERM_BUFFER];
int cur_term = 0;
struct atom_t * atoms_buf[ATOM_BUFFER];
int cur_atom = 0;
struct clause_t * clauses_buf[CLAUSE_BUFFER];
int cur_clause = 0;

%}

%pure-parser

%lex-param   { yyscan_t scanner }
%parse-param { struct clause_t *** clauses }
%parse-param { yyscan_t scanner }

%union {
  char * string;
  struct clause_t * clause;
  struct atom_t * atom;
  struct term_t * term;
  struct term_t ** terms;
  struct atom_t ** atoms;
  struct clause_t ** clauses;
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
%type <clauses> program
%start program
%%

term : TK_CONST
       { char * identifier = strdup($1);
         struct term_t * t = mk_term(Const, identifier);
         $$ = t; }
     | TK_VAR
       { char * identifier = strdup($1);
         struct term_t * t = mk_term(Var, identifier);
         $$ = t; }
     | TK_STRING
       { char * identifier = strdup($1);
         struct term_t * t = mk_term(Str, identifier);
         $$ = t; }

terms : term TK_R_RB
        { terms_buf[cur_term++] = $1; // FIXME could work back from a maximum instead of later reversing the parameter list.
          $$ = terms_buf; /* FIXME weird */}
      | term TK_COMMA terms
        { terms_buf[cur_term++] = $1;
          $$ = terms_buf; }
      | TK_R_RB
        { $$ = terms_buf; }

atom : TK_CONST TK_L_RB terms
       { char * predicate = strdup($1);
         struct atom_t * atom = mk_atom(predicate, cur_term, terms_buf);
         cur_term = 0;
         $$ = atom; }

atoms : atom TK_PERIOD
        { atoms_buf[cur_atom++] = $1;
          $$ = atoms_buf; }
      | atom TK_COMMA atoms
        { atoms_buf[cur_atom++] = $1;
          $$ = atoms_buf; }

clause : atom TK_PERIOD
         { struct clause_t * cl = mk_clause($1, 0, NULL);
           cur_atom = 0;
           $$ = cl; }
       | atom TK_IF atoms
           { struct clause_t * cl = mk_clause($1, cur_atom, atoms_buf);
             cur_atom = 0;
             $$ = cl; }

clauses : clause
          { clauses_buf[cur_clause++] = $1;
            $$ = clauses_buf; }
        | clause clauses
          { clauses_buf[cur_clause++] = $1;
            $$ = clauses_buf; }

program : clauses
          { //*clause = $1;
             clauses_buf[cur_clause] = NULL;
             *clauses = clauses_buf;
             cur_clause = 0;
          }

%%

int yyerror(struct clause_t ***clause, yyscan_t scanner, const char * error_message) {
  fprintf(stderr, "parse error: %s\n", error_message);
  return 0;
}

struct clause_t ** parse(const char * string) {
  struct clause_t ** parsed = NULL;
  yyscan_t scanner;
  YY_BUFFER_STATE state;
  if (yylex_init(&scanner)) {
    return NULL;
  }

  state = yy_scan_string(string, scanner);
  if (yyparse(&parsed, scanner)) {
    return NULL;
  }

  yy_delete_buffer(state, scanner);
  yylex_destroy(scanner);
  return parsed;
}
