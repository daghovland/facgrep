#!/bin/bash

# Constructs "follow_prec.c"

function realloc_follow ()
{
    echo "          q->${FOLLOW_PREC}_set[letterIndex] = (follow_set*) realloc(q->${FOLLOW_PREC}_set[letterIndex], sizeof(follow_set) * ( q->size_${FOLLOW_PREC}_set[letterIndex] + 1 ) );"
    echo "          foll_rq = &(q->${FOLLOW_PREC}_set[letterIndex][q->size_${FOLLOW_PREC}_set[letterIndex]]);"
    echo "          foll_rq->updates = (unsigned char *) calloc(updates_size, 1);"
}

cat <<EOF
/** Constructed automatically by create_follow_prec.sh. Do not edit **/
/* 
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

/*   Written October, 2008 by Dag Hovland  */

EOF

for FOLLOW_PREC in "follow" "prec"; do
    if [ $FOLLOW_PREC == "follow" ]; then
	FIRSTSET="first"
	LASTSET="last"
	FIRST_EXP="r1"
	LAST_EXP="value.re2"
    else
	FIRSTSET="last"
	LASTSET="first"
	FIRST_EXP="value.re2"
	LAST_EXP="r1"
    fi
    cat <<EOF
/**
   Sets correctly the members follow_set and size_follow_set in 
   the whole tree of regexp under and including re
**/
EOF
    echo "void set_$FOLLOW_PREC(regexp* root, regexp* re){"
    echo "  size_t i, j;"
    echo "  bool in$LASTSET;"
    cat <<EOF
  switch(re->type){
  case RE_EPSILON:
    break;
  case RE_ANY_CHAR:
  case RE_CHAR_CLASS:
  case RE_LETTER:
EOF
    echo "    re->size_${FOLLOW_PREC}_set = (size_t*) calloc((num_constraints+1) * ( num_pos_letters + num_epsilons ) * (alph_size+1), sizeof(size_t));"
    echo "    re->${FOLLOW_PREC}_set = (follow_set**) calloc( alph_size + 1, sizeof(follow_set*) );"
    echo "    for(i = 0; i <= alph_size; i++){"
    echo "      re->size_${FOLLOW_PREC}_set[i] = 0;"
    echo "      re->${FOLLOW_PREC}_set[0] = NULL;"
    echo "    }"
    cat <<EOF
    break;
  case RE_CHOICE:
EOF
    echo "    set_$FOLLOW_PREC(root, re->r1);"
    echo "    set_$FOLLOW_PREC(root, re->value.re2);"
cat <<EOF
    break;
  case RE_CONCAT:
EOF
    echo "    set_$FOLLOW_PREC(root, re->r1);"
    echo "    set_$FOLLOW_PREC(root, re->value.re2);"
    cat <<EOF
    for(i = 0; i < num_pos_letters; i++){
      regexp* q = pos_letters[i];
EOF
    printf  "      in%s = in_%s(re->%s, i);\n" $LASTSET $LASTSET ${FIRST_EXP}
    echo "      if(in$LASTSET){"
    printf "	for(j = 0; j < re->%s->size_%s_set; j++){\n" $LAST_EXP $FIRSTSET
    echo "	  char let_val;"
    printf "	  regexp* q1 = re->%s->%s_set[j];\n" $LAST_EXP $FIRSTSET 
    cat <<EOF
          size_t letterIndex;
	  if(q1->type == RE_ANY_CHAR){
EOF
    echo "            follow_cat_setter(root, q, q1, re, q->${FOLLOW_PREC}_set, q->size_${FOLLOW_PREC}_set, ANY_CHAR_INDEX);"
    echo "	  } else {"
    echo "            for(let_val =  q1->value.let_inf.let_val; let_val <=  q1->value.let_inf.upper; let_val++){"
    echo "	      letterIndex = getCharAlphPos(let_val);"
    echo "	      assert(getCharAlphPos(q1->value.let_inf.let_val) > 0);"
    echo "              follow_cat_setter(root, q, q1, re, q->${FOLLOW_PREC}_set, q->size_${FOLLOW_PREC}_set, letterIndex);"
    echo "	    }"
    echo "	  }"
#    realloc_follow
#    echo "	  foll_rq->next = q1;"
#    echo "	  for(c = 0; c < re->${FIRST_EXP}->num_constraints_r; c++){"
#    echo "	    regexp* constr = re->${FIRST_EXP}->constraints_r[c];"
#    cat <<EOF
#	    assert(constr->type == RE_CONSTR);
#	    if(position_incl(constr, q)){
#	      con_no = constr->value.constr.pos_constraints;
#	      assert(root->constraints_r[con_no] == constr);
#	      foll_rq->updates[con_no] = FOLLOW_RES;
#	    }
#	  }
#EOF
#    echo "	  q->size_${FOLLOW_PREC}_set[letterIndex]++;"
    cat <<EOF
	}
      }
    }
    break;
  case RE_CONSTR:
EOF
    echo "    set_${FOLLOW_PREC}(root, re->r1);"
    cat <<EOF
    for ( i = 0; i < num_pos_letters; i++){
      regexp* q = pos_letters[i];
EOF
    echo "      if( in_$LASTSET(re->r1, i) ) {"
    echo "	for(j = 0; j < re->r1->size_${FIRSTSET}_set; j++){"
    echo "	  char let_val;"
    echo "          regexp* q1 = re->r1->${FIRSTSET}_set[j];"
    echo "	  size_t letterIndex;"
    echo "        if(q1->type == RE_ANY_CHAR){"
    echo "          assert(letterIndex >= 0);"
    echo "            follow_constr_setter(root, q, q1, re, q->${FOLLOW_PREC}_set, q->size_${FOLLOW_PREC}_set, ANY_CHAR_INDEX);"
    echo "	  } else {"
    echo "            for(let_val =  q1->value.let_inf.let_val; let_val <=  q1->value.let_inf.upper; let_val++){"
    echo "	      letterIndex = getCharAlphPos(let_val);"
    echo "	      assert(getCharAlphPos(q1->value.let_inf.let_val) > 0);"
    echo "              follow_constr_setter(root, q, q1, re, q->${FOLLOW_PREC}_set, q->size_${FOLLOW_PREC}_set, letterIndex);"
    echo "	    }"
    echo "	  }"
    cat <<EOF
	}
      }
    }
    break;
  default:
    assert(0);
    break;
  }
  return;
}

EOF
done
