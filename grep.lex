%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "grep.h"
#include "malloc.h"
%}

%option 8bit
%option   warn nodefault
%option nounput

%%

\\e                { return EPSILON; }
\[.-.\]        { char s, e; sscanf(yytext,"[%c-%c]",&s, &e); yylval.re = create_re_char_class(s,e); return CHAR_CLASS; }
\.                 { return ANY_CHAR; }
\\.                {sscanf(yytext,"\\%c",&(yylval.chr)); return LETTER; }
\{[0-9]+,[0-9]+\}  { unsigned int l, u ; sscanf(yytext, "{%i,%i}",&l,&u); yylval.con = create_constraint(l,u); return CONSTR;}
\{[0-9]+,\}       { unsigned int l ; sscanf(yytext, "{%i,}",&l); yylval.con = create_constraint(l,0); return CONSTR;}
\{[0-9]+\}  { unsigned int l ; sscanf(yytext, "{%i}",&l); yylval.con = create_constraint(l,l); return CONSTR;}
\*                 { yylval.con = create_constraint(0,0); return CONSTR;}
\+                 { yylval.con = create_constraint(1,0); return CONSTR;}
\?                 { yylval.con = create_constraint(0,1); return CONSTR;}
\(                 { return LPAREN; }
\)                 { return RPAREN; }
\|                 { return CHOICE; }
[^\{^\}^\(^\)]        { sscanf(yytext,"%c",&(yylval.chr)); return LETTER; }
[\{\}]                  { fprintf(stderr, "'{' and '}' can only be used in constraints, as '{1,30}'. Use '\\{' and '\\}' for literal '{' and '}'.\n"); exit(EXIT_FAILURE);} 
.                    {fprintf(stderr, "Error with symbol %s\n", yytext); exit(EXIT_FAILURE);}
%%

