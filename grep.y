/* grep.y  

   Copyright 2008 University of Bergen

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA  02110-1301, USA */

/*   Written July-October, 2008 by Dag Hovland  */


%{
#include <stdio.h>
#include <wchar.h>
#include "fac.h"
#include "grep.h"
#include "malloc.h"
#include "init.h"
#include <string.h>
#include <getopt.h> 

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

  
  extern int mainloop(bool, bool, bool, bool, bool, int, size_t*, regexp*, char*, bool, bool, bool, char*, FILE *);
  
  extern int fileno(FILE*);
  
  regexp* result;
  void yyerror(char*);
  int yylex();
  
  
%}
%union{
  constraint con;
  regexp*     re;
  char        chr;
}

%token <chr> LETTER
%token <con> CONSTR
%token EPSILON
%token LPAREN
%token RPAREN    
%token CHOICE
%token ANY_CHAR
%token <re> CHAR_CLASS

%type  <re> exp
%type  <re> lexp
%type  <re> expr

    
%left PLUS 

%%

line:       exp { result = $1;}
          ;
exp:        expr { $$ = $1; }
           | exp CHOICE expr { $$ = create_re_choice($1, $3);}
           |  { $$ = create_re_epsilon();}
          ;
expr:       lexp { $$ = $1; }
           | lexp expr { $$ = create_re_cat($1, $2);}
          ;
lexp:       LETTER { $$ = create_re_letter($1); }
          | ANY_CHAR { $$ = create_re_any_char(); }
          | CHAR_CLASS { $$ = $1; }
          | EPSILON { $$ = create_re_epsilon(); }
          | lexp CONSTR { $$ = create_re_cons($1, $2); }
          | LPAREN exp RPAREN { $$ = $2; }
          ;
%%

#include "lex.yy.c"

void yyerror(char* s){
  printf("Error in parsing regular expression: %s\n",s);
  exit(2);
}

int main(int argc, char ** argv){
  YY_BUFFER_STATE bs;
  int grep_retval = EXIT_FAILURE;
  size_t* bad_char_skip;
  char* regexp_text;
  size_t re_length;
  const char* const short_options = "hgvxcne:E";
  const struct option long_options[] = {
    { "help", 0, NULL, 'h' },
    { "verbose", 0, NULL, 'g' },
    { "invert-match", 0, NULL, 'v' },
    { "line-regexp", 0, NULL, 'x' },
    { "count", 0, NULL, 'c' },
    { "line-number", 0, NULL, 'n' },
    { "regexp", 1, NULL, 'e' },
    { "extended-regexp", 0, NULL, 'E' },
    { NULL, 0, NULL, 0 }
  };
  bool verbose = false;
  int next_option;
  bool line_regexp = false;
  int opt_counter;
  bool counter_unamb, counter_unamb_rev;
  bool invert_match = false;
  bool print_line_numbers = false;
  bool count_hits = false;
  bool print_file_names = false;

  spc_sanitize_files();

  if(argc < 2){
     fprintf(stderr,"Usage: %s <regexp>\n", argv[0]);
     return EXIT_FAILURE;
  }
  regexp_text = NULL;

  do {
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);
    switch(next_option)
      {
      case 'h':
	fprintf(stdout,"Usage: %s <regexp>\n", argv[0]);
	return EXIT_SUCCESS;
      case 'v':
	invert_match = true;
	break;
      case 'g':
	verbose = true;
	break;
      case 'c':
	count_hits = true;
	break;
      case 'n':
	print_line_numbers = true;
	break;
      case 'x':
	line_regexp = true;
	break;
      case 'E':
	break;
      case 'e':
	regexp_text = optarg;
	break;
      case '?':
	fprintf(stderr, "Invalid option\n");
	return EXIT_FAILURE;
      case -1:
	break;
      default:
	abort();
	
      }
  } while(next_option != -1);
  opt_counter = optind;
  if(regexp_text == NULL)
    regexp_text = argv[opt_counter++];
  print_file_names = (opt_counter + 1 < argc);

  re_length = strlen(regexp_text);
  pre_parse_init(re_length);

  bs = yy_scan_bytes(regexp_text, re_length);

  yyparse();

  yy_delete_buffer(bs);

  if(result != 0){
    
    result = mult_constr(result);
    result = simpl_eps(result);
    
    post_parse_init(result);
    
    if(!constraint_normal_form(result)){
      fprintf(stderr, "The expression is not in constraint normal form! The translation to constraint normal form could be super-polynomial . \n");
      return EXIT_FAILURE;
    };
    
    set_first_last(result);
    set_follow(result, result);
    set_prec(result, result);
  
    if(verbose){
      printf("r");
      fprintf_space(stdout, 1);
      fprint_regexp(stdout, result);
      printf("\nµ(r)");
      fprintf_space(stdout, 4);
      fprint_mu_re(stdout, result);
      printf("\nsym(r)");
      fprintf_space(stdout, 6);
      fprintf_alphabet(stdout);
      printf("\nfirst(r)");
      fprintf_space(stdout, 8);
      printf("{");
      fprintf_first(stdout, result->first_set, 0, result->size_first_set);
      printf("}");
      fprintf_follow_sets(stdout, result);    
      
      fprintf_delta(stdout, result);
    }
    counter_unamb = counter_unambiguous(result);
    counter_unamb_rev = counter_unambiguous_rev(result);
    //counter_unamb_rev = false;
    if(!counter_unamb && !counter_unamb_rev){
      fprintf(stderr, "The expression is not counter-unambiguous! The recognition procedure may reject sentences that are in the language. \n");
      return EXIT_FAILURE;
    }

    fac_init(result);
    bad_char_skip = counter_unamb_rev ? create_bad_char_skip(result) : create_bad_char_skip_rev(result);
    assert(bad_char_skip != 0);
    do{
      bool init_ok;
      if(opt_counter == argc)
	init_ok = init_readstd();
      else
	{
	  struct stat buf;
	  char* filename = argv[opt_counter];
	  if(stat(filename, &buf) != 0)
	    continue;
	  init_ok = init_readfile(filename);
	}
      if(init_ok)
	grep_retval = mainloop(
			     counter_unamb, 
			     counter_unamb_rev, 
			     verbose, 
			     line_regexp, 
			     invert_match, 
			     grep_retval, 
			     bad_char_skip, 
			     result, 
			     regexp_text, 
			     count_hits, 
			     print_line_numbers, 
			     print_file_names, 
			     argv[opt_counter],
			     stdout
			     );
    } while(++opt_counter < argc);
    free_bad_char_skip(bad_char_skip);
  }
  free_mem(); 
  end_readfile();
  return grep_retval;
}

