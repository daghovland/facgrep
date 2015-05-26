/* Fac.h   - header for Finita Automata with Counters 

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

/*   Written July, 2008 by Dag Hovland  */

#ifndef FAC_HH
#define FAC_HH
#include "grep.h"

regexp** firstStateLetter;
regexp** lastStateLetter;

void fac_init(regexp*);
fac_state* fac_delta(regexp*, fac_state*, char);
fac_state* fac_delta_prec(regexp*, fac_state*, char);
/*@null@*/ fac_state* fac_delta_qi(regexp*, char);
/*@null@*/ fac_state* fac_delta_last(regexp*, char);
bool fac_final_rev(regexp*, fac_state*);
bool fac_final(regexp*, fac_state*);
int recognize_word_fac(char*, char*, regexp*);
int recognize_word_fac_rev(char*, char*, regexp*);
bool recognize_whole_word_rev(char*, char*, regexp*);
bool recognize_whole_word(char*, char*, regexp*);
bool recognize_whole_word_rev_verbose(char*, char*, regexp*, FILE*);
bool recognize_whole_word_verbose(char*, char*, regexp*, FILE*);
#endif
