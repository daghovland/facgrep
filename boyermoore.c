/* boyermoore.c   

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

/*   Written August, 2008 by Dag Hovland  */
#include "grep.h"
#include "regexp.h"
#include "fac.h"
#include <unistd.h>
#include <limits.h>

#define DIR_L2R 1
#define DIR_REV 2


size_t  calc_bad_char_skip(regexp*, regexp*);
size_t  calc_bad_char_skip_rev(regexp*, regexp*);
size_t * create_bad_char_skip_gen(regexp*, size_t (*) (regexp*, regexp*));
size_t calc_bad_char_skip_gen(regexp*, regexp*, int);

size_t * create_bad_char_skip_gen(regexp* re, size_t (*calc_bad_skip) (regexp*, regexp*)){
  int alph_ind;
  size_t de_let, j, * bad_char_skip;
  ssize_t i;
  size_t max_skip;
  bad_char_skip = calloc((size_t) SCHAR_MAX - SCHAR_MIN + 1, sizeof(size_t));
  bad_char_skip += -(SCHAR_MIN);

  max_skip = re->min_word_length;
  for(j = 0; j < num_pos_letters; j++){
    if(pos_letters[j]->type == RE_ANY_CHAR){
      de_let = calc_bad_skip(re, pos_letters[j]);
      if(de_let < max_skip)
	max_skip = de_let;
    }
  }
  for(i = SCHAR_MIN; i <= (size_t) SCHAR_MAX; i++){
    assert(bad_char_skip[i] == 0);
    bad_char_skip[i] = max_skip;
  }
  
  for(j = 0; j < num_pos_letters; j++){
    if(pos_letters[j]->type != RE_ANY_CHAR){
      de_let = calc_bad_skip(re, pos_letters[j]);
      pos_letters[j]->skip_forward = de_let;
      for( alph_ind = pos_letters[j]->value.let_inf.let_val; alph_ind <= pos_letters[j]->value.let_inf.upper; alph_ind++){
	assert(alph_ind >= SCHAR_MIN && alph_ind <= SCHAR_MAX);
	if(de_let < bad_char_skip[alph_ind])
	  bad_char_skip[alph_ind] = de_let;
      }
    }      
  }
  
  return bad_char_skip;
}

size_t * create_bad_char_skip_rev(regexp* re){
  return create_bad_char_skip_gen(re, calc_bad_char_skip_rev);
}

size_t * create_bad_char_skip(regexp* re){
  return create_bad_char_skip_gen(re, calc_bad_char_skip);
}

size_t calc_bad_char_skip(regexp* re, regexp* pos){
  return calc_bad_char_skip_gen(re, pos, DIR_L2R);
}

size_t calc_bad_char_skip_rev(regexp* re, regexp* pos){
  return calc_bad_char_skip_gen(re, pos, DIR_REV);
}

size_t calc_bad_char_skip_gen(regexp* re, regexp* pos, int dir){
  assert(dir == DIR_REV || dir == DIR_L2R);
  assert(DIR_REV != DIR_L2R);
  switch(re->type){
  case RE_ANY_CHAR:
  case RE_CHAR_CLASS:
  case RE_LETTER:
    assert (re == pos);
    return 0;
  case RE_CONCAT:
    assert(pos->tpos.nlist > re->tpos.nlist);
    if(pos->tpos.list[re->tpos.nlist] == POS_LEFT) {
      return calc_bad_char_skip_gen(re->r1, pos, dir) + (dir == DIR_L2R) ? re->value.re2->min_word_length : 0;
    } else {
      assert(pos->tpos.list[re->tpos.nlist] == POS_RIGHT);
      return calc_bad_char_skip_gen(re->value.re2, pos, dir) + (dir == DIR_REV) ? re->r1->min_word_length : 0;
    }
    assert(false);
  case RE_CHOICE:
    assert(pos->tpos.nlist > re->tpos.nlist);
    if(pos->tpos.list[re->tpos.nlist] == POS_LEFT)
      return calc_bad_char_skip_gen(re->r1, pos, dir);
    else {
      assert(pos->tpos.list[re->tpos.nlist] == POS_RIGHT);
      return calc_bad_char_skip_gen(re->value.re2, pos, dir);
    }
    assert(false);
  case RE_CONSTR:
    assert(pos->tpos.nlist > re->tpos.nlist);
    assert(pos->tpos.list[re->tpos.nlist] == POS_LEFT);
    return calc_bad_char_skip_gen(re->r1, pos, dir);
  default:
    assert(false);
  }
  assert(false);
  return 0;
}


#include "findword.c"



void free_bad_char_skip(size_t* bad_char_skip){
  bad_char_skip += SCHAR_MIN;
  if(bad_char_skip != 0)
    free(bad_char_skip);
}

