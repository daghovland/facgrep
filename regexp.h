/* regexp.c   

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

#ifndef REGEXP_INC
#define REGEXP_INC

#define RE_CHOICE 501
#define RE_LETTER 502
#define RE_CONSTR 503
#define RE_CONCAT 504
#define RE_EPSILON 506
#define RE_ANY_CHAR 507
#define RE_CHAR_CLASS 508


#define ANY_CHAR_INDEX 0

#define MAX_CHAR_VAL 512
#define UNUSED_CHAR 0

#define STATUS_PRE_INIT 1
#define STATUS_PRE_MEM_INIT 2
#define STATUS_MEM_INIT 4
#define STATUS_POST_INIT 8

#include <stdio.h>
#include <stdbool.h>
#include "grep.h"

// String length of regular expression
size_t re_len;





/**
   The position in a regular expression

**/
#define POS_LEFT 1
#define POS_RIGHT 2
#define POS_LOWER 2
#define POS_UPPER 3


typedef struct expression_t {
  /** Keeps track of the alphabet
      charAlphPos must be as large as the number of characters in the character set, at the moment MAX_CHAR_VAL
      the alphabet is indexed starting at 1. This is because 0 must be the value for a not seen character
      in charAlphPos
  **/
  char * alphabet;
  size_t alph_buf_size;
  size_t alph_size;
  
  // The alphabet of the marked expressions
  regexp** pos_letters;
  size_t pos_letters_size;
  size_t num_pos_letters;
  size_t cur_pos_letters;
  
  size_t num_constraints;
  
  /** List of positions of letters **/
  re_pos* symchar;

  regexp* root;

  // Total number of positions in the regular expressions
  size_t num_pos;
  // Keeps track of whether correct ordering of initialization is followed
  size_t status_order;



  size_t regexp_size;
  size_t re_pos_size;
  size_t size_follow_size;
  size_t first_last_set_size;
  size_t follow_set_size;
  size_t updates_size;
  size_t constraints_r_size;
  size_t fac_state_size;
  
  size_t max_pos;

} expression;
  
/** Keeps track of the alphabet
    charAlphPos must be as large as the number of characters in the character set, at the moment MAX_CHAR_VAL
    the alphabet is indexed starting at 1. This is because 0 must be the value for a not seen character
    in charAlphPos
**/
char * alphabet;
size_t alph_buf_size;
size_t alph_size;

// The alphabet of the marked expressions
regexp** pos_letters;
size_t pos_letters_size;
size_t num_pos_letters;
size_t cur_pos_letters;

size_t num_constraints;

/** List of positions of letters **/
re_pos* symchar;


regexp* create_node_regexp(regexp*, unsigned int);
regexp* create_bin_regexp(regexp*, regexp*, size_t);
size_t getCharAlphPos(char);
void setCharAlphPos(char, size_t);

void fprint_regexp(FILE*, regexp*);
void fprint_mu_re(FILE*, regexp*);
regexp* regexp_choice(regexp*, regexp*);
regexp* regexp_concat(regexp*, regexp*);
regexp* regexp_constr(regexp*, constraint*);
regexp* regexp_letter(char);

void free_regexp(regexp*);

void* get_buf(size_t);

size_t* charAlphPos;

/**
   initializes structures in the regexo
**/
void init_regexp(regexp*);
void init_regexp_mem(regexp*);
size_t copy_constraints_r(regexp**, regexp*);


void pre_init(int);

/**
   Finds recursively the "first" of the regular expression
**/

void  fprintf_position(FILE*, re_pos*);
void  print_int_array(size_t*, size_t);
void fprintf_follow_set(FILE*, regexp*, regexp**);


// Used for error-printing
char buf[200];



#endif
