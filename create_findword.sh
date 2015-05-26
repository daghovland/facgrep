#!/bin/bash

# Constructs "findword.c"
# shall contain one function for each variation of the main loop.

cat <<EOF
/** Constructed automatically by create_findword.sh. Do not edit **/
/* findword.c

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
for FUNC_NAME in "find_word" "find_word_rev"
do
    printf "\n\n\n"
    printf 'int %s(regexp* re, size_t* bad_char_skip, char* line, size_t line_len){\n' $FUNC_NAME
    cat <<EOF
  unsigned int k;
  char *s, *e, *i, *end, endval;
EOF
    if [ ${FUNC_NAME} == "find_word_rev" ]; then echo "  char *line_end;"; fi
    cat <<EOF
  int retval;
  
  if(line_len < re->min_word_length)
    return -1;
  if(re->nullable)
    return 0;
  
EOF
    if [ ${FUNC_NAME} == "find_word" ]; then
	cat <<EOF
  s = line + re->min_word_length - 1;
  e = line + line_len;
  //memset(e, re->last_set[0]->value.let_inf.let_val, re->min_word_length); 
  end = e + re->min_word_length;
  endval = re->last_set[0]->value.let_inf.let_val;
  for(i = e; i <= end; i++)
    *i = endval;
EOF
    else
	cat <<EOF
  s = line + line_len;
  e = line + re->min_word_length;
  line_end = line + line_len + re->min_word_length;
  //memset(line, re->first_set[0]->value.let_inf.let_val, re->min_word_length);  
  end = line + re->min_word_length;
  endval = re->first_set[0]->value.let_inf.let_val;
  for(i = line; i < end; i++)
    *i = endval;
EOF
    fi
    echo '  retval = -1;'
    if [ $FUNC_NAME == "find_word" ]; then
	echo '  while(s < e && retval < 0){'
    else
	echo '  while(s >= e && retval < 0){'
    fi
    cat <<EOF
    k = bad_char_skip[ (int) *s ];
    while(k){
EOF
    for I in {1,2,3}; do
	if [ ${FUNC_NAME} == "find_word" ]; then
	    echo  "      k = bad_char_skip[ (int) *(s += k) ];"
	else
	    echo  "      k = bad_char_skip[ (int) *(s -= k) ];"
	fi
    done
    echo '    }'
    if [ ${FUNC_NAME} == "find_word" ]; then
	cat <<EOF 
    if( s >= e )
      break;
    line[line_len] = 0;
    retval = recognize_word_fac_rev(line, ++s, re);
    line[line_len] = endval;
EOF
    else
	cat <<EOF
    if( s < e )
      break;
    //line[re->min_word_length] = 0;
    retval = recognize_word_fac(s--, line_end, re);
    //line[re->min_word_length] = endval;
EOF
fi
cat <<EOF
  }
  return retval;
}
EOF
done
