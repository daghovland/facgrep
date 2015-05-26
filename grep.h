/* grep.h

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

/*   Written July and August, 2008 by Dag Hovland  */

#define NDEBUG
#ifndef GREP_INC_DEF
#define GREP_INC_DEF
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/** Represents constraints, kleene * and the '+'
    unlimited upper constraint is coded by upper = 0
 **/

struct constraint_t {
  unsigned int lower;
  unsigned int upper;
  size_t pos_constraints;
};

typedef struct constraint_t constraint;

#define FOLLOW_RES 2
#define FOLLOW_INC 1
#define FOLLOW_NIL 0



/** Creates a constraint structure **/
constraint create_constraint(unsigned int, unsigned int);

/**
   The position in a regular expression

 **/
#define POS_LEFT 1
#define POS_RIGHT 2
#define POS_LOWER 2
#define POS_UPPER 3

struct re_pos{
  size_t*  list;
  size_t   nlist;
};

typedef struct re_pos re_pos;


typedef struct follow_set_t {
  void * next;
  unsigned char * updates;
} follow_set;

/**
   Represents a regular expression internally
 **/

struct regexp_t
{
  unsigned int type;
  struct regexp_t *r1;
  union {
    struct letter_t{
      char let_val;
      // The index in the array pos_letters where this regexp is. Only set if type = RE_LETTER or RE_ANY_CHAR or RE_CLASS_CHAR
      size_t pos_letters_pos;
      char upper;
    } let_inf;
    constraint constr;
    struct regexp_t *re2;
  } value;
  /* Position in tree of regexp */
  re_pos tpos;
  /* set to true in re_alphabet if the expression is nullable */
  bool nullable;

  struct regexp_t ** first_set;
  size_t size_first_set;

  struct regexp_t ** last_set;
  size_t size_last_set;

  /**
      Here is stored the follow set of the expression
      This is a mapping from each position/regexp containing a letter to
      a set of pairs of a regexp and a parital mapping from constraints to res, ing
      Note follow_set[0], if non-empty contains the "any char" (.) follow set.
   **/
  follow_set ** follow_set;
  size_t * size_follow_set;

  /**
      Here is stored the "prececding set" of the expression
      This is a mapping from each position/regexp containing a letter to
      a set of pairs of a regexp and a parital mapping from constraints to res, ing
      Note prec_set[0], if non-empty contains the "any char" (.) prec set.
   **/
  follow_set ** prec_set;
  size_t * size_prec_set;

  // This is R(r), the set of constraints "included" in this subexpression
  struct regexp_t ** constraints_r;
  size_t num_constraints_r;

  /** The length of the shortest word in the language of the expression **/
  size_t min_word_length;

  /** The number of characters to skip forwards in string when matching
      with the reverse regexp, as usual, and to front skip when matching irregularly
   **/
  size_t skip_forward;
};

typedef struct regexp_t regexp;

typedef struct fac_state_t {
  regexp* q;
  unsigned int * counters;
} fac_state;

bool position_incl(regexp *, regexp*);

size_t * create_bad_char_skip(regexp*);
size_t * create_bad_char_skip_rev(regexp*);
void free_bad_char_skip(size_t*);
int find_word(regexp*, size_t*, char*, size_t);
int find_word_rev(regexp*, size_t*, char*, size_t);

bool counter_unambiguous(regexp*);
bool counter_unambiguous_rev(regexp*);

regexp* create_regexp(unsigned int);
regexp* create_node_regexp(regexp*, size_t);
regexp* create_bin_regexp(regexp*, regexp*, size_t);

regexp* create_re_choice(regexp*, regexp*);
regexp* create_re_cat(regexp*, regexp*);
regexp* create_re_any_star(regexp*);
regexp* create_re_any_char();
regexp* create_re_cons(regexp*, constraint);
regexp* create_re_letter(char);
regexp* create_re_char_class(char, char);
regexp* create_re_epsilon(void);
void fprint_regexp(FILE* , regexp*);
void fprint_mu_re(FILE* , regexp*);
regexp* regexp_choice(regexp*, regexp*);
regexp* regexp_concat(regexp*, regexp*);
regexp* regexp_constr(regexp*, constraint*);


void free_regexp(regexp*);


/**
   initializes structures in the regexo
 **/
void init_regexp(regexp*);
void init_regexp_mem(regexp*);
regexp* mult_constr(regexp*);
bool constraint_normal_form(regexp*);
regexp* simpl_eps(regexp*);

/**
   Finds recursively the "first" of the regular expression
 **/

void set_first_last(regexp*);
void set_follow(regexp*, regexp*);
void set_prec(regexp*, regexp*);
void  fprintf_first(FILE * f, regexp**, size_t, size_t);
void fprintf_follow_sets(FILE*, regexp*);
void fprintf_alphabet(FILE*);
void fprintf_delta(FILE*, regexp*);


void pre_parse_init(size_t);
void post_parse_init(regexp*);

void free_mem(void);
void fprintf_space(FILE *, size_t);

/* From readfile.c */
void end_readfile(void);
char* nextline(size_t, size_t*, size_t);
bool init_readfile(char*);
bool init_readstd();
bool get_file_finished();
#endif
