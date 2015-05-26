#!/bin/bash

# Constructs "fac_gen.c"
# shall contain one function for each variation of the main loop.
function printDeltaBody()
{
    printf "  foll_ql = state->q->%s_set[%s];\n" $SETNAME ${SET_INDEX}
    printf "  foll_size = state->q->size_%s_set[%s];\n" $SETNAME ${SET_INDEX}
    
    cat<<EOF 
  for(i = 0; i < foll_size; i++){
    found = true;
    for(c = 0; c < num_constraints; c++){
      constraint constr = re->constraints_r[c]->value.constr;
      if(
	 ( foll_ql[i].updates[c] == FOLLOW_INC && constr.upper != 0 && (state->counters[c] + 1 >= (constr.upper)))
	 || ( foll_ql[i].updates[c] == FOLLOW_RES && (state->counters[c] + 1 < (constr.lower)))
	 ){
	found = false;
	break;
      }
    }
    if(found){
      state->q = (regexp*) foll_ql[i].next;
      for(u = 0; u < num_constraints; u++){
	if(foll_ql[i].updates[u] == FOLLOW_INC)
	  state->counters[u]++;
	else if( foll_ql[i].updates[u] == FOLLOW_RES)
	  state->counters[u] = 0;
      }
EOF
    
    cat <<EOF
      return state;
    }
  }
EOF
}
cat <<EOF
/** Constructed automatically by create_fac.sh. Do not edit **/
/* fac_gen.c   - Finita Automata with Counters 

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
#include "regexp.h"
#include "fac.h"
#include <strings.h>
#include <limits.h>

fac_state *q;

// Contains zeroes. Used to initialize the counters in fac_delta_last and fac_delta_qi
unsigned int * zero_counter;

/* If one of the first/last set is an ANY_CHAR, index of this position in the last/first set */
int anychar_first;
int anychar_last;


/** Prints state during verbose matching **/
void fprintf_fac_state(FILE* f, bool qi, regexp* re){
  size_t i;
  bool first_print = true;
  if(qi)
    fprintf(f, "\n(qI, {");
  else {
    fprintf(f, "(<");
    fprintf_position(f, &(q->q->tpos));
    fprintf(f, ">, {");
  }
  for(i = 0; i < num_constraints; i++){
    if(!first_print)
      fprintf(f, ",");
    first_print = false;
    fprintf(f, "<");
    fprintf_position(f, &(re->constraints_r[i]->tpos));
    fprintf(f, ">=%i", q->counters[i]);
  }
  fprintf(f, "})\n");
}
EOF

for DELTA_FUNC in "delta_qi" "delta_last"
do
    for VERBOSE in ""
    do
	if [ ${DELTA_FUNC} = "delta_qi" ] ; then
	    SIZESET="re->size_first_set"
	    SETNAME="re->first_set"
	    FIRST_LAST="first"
	else
	    SIZESET="re->size_last_set"
	    SETNAME="re->last_set"
	    FIRST_LAST="last"
	fi
	cat <<EOF

/**
   delta(qi, letter)
   This is the delta function applied to initial state and counter
   Also initializes the memory structures needed
**/

EOF
	if [ ${VERBOSE} ] ; then
	    printf "fac_state* fac_%s_verbose(regexp* re, char letter, FILE * f){\n" ${DELTA_FUNC}
	else
	    printf "fac_state* fac_%s(regexp* re, char letter){\n" ${DELTA_FUNC}
	fi
	cat <<EOF
  //size_t i;
  
  // memset(q->counters, 0, re->num_constraints_r * sizeof ( int ) );
  memcpy(q->counters, zero_counter, re->num_constraints_r * sizeof ( int ) );
  //  for(i = 0; i < re->num_constraints_r; i++)
  //  q->counters[i] = 0;
EOF
	echo "  if(anychar_${FIRST_LAST} >= 0){"
	echo "    q->q = ${SETNAME}[anychar_${FIRST_LAST}];"
	echo "    return q;"
	echo "  }"
	echo "  if(${FIRST_LAST}StateLetter[(int) letter] != 0){" 
	echo "    q->q = ${FIRST_LAST}StateLetter[(int) letter];"
	cat <<EOF
    return q;
  }
  return (fac_state*) 0;
}

EOF
    done
done

cat <<EOF

void fac_init(regexp* re){
  size_t i;
  char let_val;
  q = (fac_state*) get_buf(sizeof(fac_state));
  q->counters = (unsigned int*) calloc(re->num_constraints_r, sizeof(unsigned int));
  zero_counter = (unsigned int*) calloc(re->num_constraints_r, sizeof(unsigned int));
EOF
for FIRSTLAST in "first" "last"
do
    echo "  ${FIRSTLAST}StateLetter = calloc(SCHAR_MAX - SCHAR_MIN + 1, sizeof(regexp*));"
    echo "  ${FIRSTLAST}StateLetter -= SCHAR_MIN;"
    echo "  anychar_${FIRSTLAST} = -1;"
    echo "  for(i = 0; i < re->size_${FIRSTLAST}_set; i++){"
    echo "    if(re->${FIRSTLAST}_set[i]->type == RE_ANY_CHAR)"
    echo "      anychar_${FIRSTLAST} = i;"
    echo "    else {"
    echo "      for(let_val = re->${FIRSTLAST}_set[i]->value.let_inf.let_val; let_val <= re->${FIRSTLAST}_set[i]->value.let_inf.upper; let_val++)"
    echo "        ${FIRSTLAST}StateLetter[(int) let_val] = re->${FIRSTLAST}_set[i];"
    echo "    }"
    echo "  }"
done
cat <<EOF
 }


EOF
for DELTA_FUNC in "delta" "delta_prec"
do
    for VERBOSE in "" "verbose"
    do
	if [ ${DELTA_FUNC} = "delta" ]; then
	    SETNAME="follow"
	else
	    SETNAME="prec"
	fi
	cat <<EOF
/**
   This is the delta function
**/
EOF
	if [ ${VERBOSE} ] ; then
	    printf "fac_state* fac_%s_verbose(regexp* re, fac_state* state, char letter, FILE * f){\n" $DELTA_FUNC
	else	
	    printf "fac_state* fac_%s(regexp* re, fac_state* state, char letter){\n" $DELTA_FUNC
	fi
	cat <<EOF
  size_t i, c, u;
  bool found;
  follow_set* foll_ql;
  size_t foll_size;
  size_t let_no;
EOF
	SET_INDEX="0"
	printDeltaBody
	cat <<EOF
  let_no = getCharAlphPos(letter);
  if(getCharAlphPos(letter) == 0)
    return (fac_state*) 0;
EOF
	SET_INDEX="let_no"
	printDeltaBody
	cat <<EOF
    return (fac_state*) 0;
}

EOF
    done
done

for FINAL_FUNC in "final" "final_rev"
do
    printf "bool fac_%s(regexp* re, fac_state* state){\n" $FINAL_FUNC
    if [ ${FINAL_FUNC} = "final" ]; then
	SETNAME="last"
    else
	SETNAME="first"
    fi
    cat <<EOF
  bool final = false;
  size_t i, size_set;
  regexp** set;

EOF
    printf "  set = re->%s_set;\n" $SETNAME
    printf "  size_set = re->size_%s_set;\n" $SETNAME
    cat <<EOF
  for(i = 0; i < size_set; i++){
    if(state->q == set[i]){
      final = true;
      break;
    }
  }
  if(!final)
    return false;
  for(i = 0; i < re->num_constraints_r; i++){
    if( position_incl ( re->constraints_r[i], state->q ) && ( state->counters[i] + 1 < re->constraints_r[i]->value.constr.lower ) )
      return false;
  }
  return true;
}

EOF
done

for VERBOSE in "" "verbose"
do
    if [ "${VERBOSE}" ] ; then 
	func_name_modifier="_verbose"
	func_arg_mod=", outfile"
	func_param_mod=", FILE * outfile"
    else
	func_name_modifier=""
	func_arg_mod=""
	func_param_mod=""
    fi
    for func_name in {"word_fac","word_fac_rev","whole_word_rev","whole_word"}
    do
	if [ "$func_name" = "word_fac" -o "$func_name" = "whole_word" ]
	then
	    delta_qi_func="fac_delta_qi(fac, *(stringstart++) )"
	    delta_func="fac_delta( fac, fs, *(stringstart++) )"
	    final_func="fac_final(fac, fs)"
	else
	    delta_qi_func="fac_delta_last(fac, *(--stringend) )"
	    delta_func="fac_delta_prec( fac, fs, *(--stringend))"
	    final_func="fac_final_rev(fac, fs)"
	fi
	if [  "$func_name" = "whole_word"  -o  "$func_name" = "whole_word_rev"  ] ; then
	    MODIFIER=""
	    RET_TYPE="bool"
	    RET_FAIL="false"
	else
	    MODIFIER=" - 1"
	    RET_TYPE="int"
	    RET_FAIL="-1"
	fi
	cat <<EOF

/**
   Recognizes a word using the regexp* structure created from it
   Exact recognition - the whole word must match
**/
EOF
	printf "%s recognize_%s%s(char* stringstart, char* stringend, regexp* fac %s ){\n" "${RET_TYPE}"  "${func_name}" "${func_name_modifier}" "${func_param_mod}"
	cat <<EOF
  fac_state* fs;
  
  if(stringstart == stringend)
EOF
	echo "    return fac->nullable$MODIFIER;"
	echo "  fs = $delta_qi_func;"
	if [ ${VERBOSE} ] ; then
	    echo '    fprintf_fac_state(outfile, true, fac);'
	fi
	echo "  while(stringend > stringstart){"
	echo "    if(fs == 0)"
	printf "      return %s;\n" ${RET_FAIL}
	if [ ${RET_TYPE} == "int" ] ; then
	    echo "    if ( ${final_func} )"
	    echo   "      return stringend - stringstart;"
	fi
	echo "    fs = ${delta_func};"
	if [ ${VERBOSE} ] ; then
	    echo '    fprintf_fac_state(outfile, false, fac);'
	fi
	cat <<EOF
  }
  if(fs != 0)
EOF
	echo "    return ${final_func}${MODIFIER};"
	echo "  return ${RET_FAIL};"
	echo "}"
    done
done
