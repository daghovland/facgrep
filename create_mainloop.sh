#!/bin/bash

# Constructs "mainloop.c"
# shall contain one function for each variation of the main loop.

function printHitLine()
{
    echo "      grep_retval = EXIT_SUCCESS;"
    if [ $COUNT_HITS == "true" ]; then 
	echo "      hit_counts++;"
    else
	if [ $FILE_NAMES == "true" ]; then
	    echo '      printf("%s:", filename);'
	fi
	if [ $LINE_NUMBER == "true" ]; then
	    echo '      printf("%i:%s\r\n", line_number, line);'
	else
	    echo '      printf("%s\r\n", line);'
	fi
    fi
}

cat <<EOF
/** Constructed automatically by create_mainloop.sh. Do not edit **/
#include <stdio.h>
#include <wchar.h>
#include "fac.h"
#include "grep.h"
#include "malloc.h"
#include "init.h"

EOF

PARAMLIST="(bool counter_unamb, bool counter_unamb_rev, int grep_retval, size_t* bad_char_skip, regexp* result, char* regexp_text, bool count_lines, bool print_file_names, char* filename, FILE * outfile){\n"
MAINFUNCTION="int mainloop(bool counter_unamb, bool counter_unamb_rev, bool verbose, bool line_regexp, bool invert_match, int grep_retval, size_t* bad_char_skip, regexp* result, char* regexp_text, bool count_hits, bool print_line_numbers, bool print_file_names, char* filename, FILE * outfile){\n\n"
for COUNTER_UNAMB in "true" "false"
do
    for COUNTER_UNAMB_REV in "true" "false"
    do
	for VERBOSE  in "false" "true"
	do
	    for LINE_REGEXP  in "true" "false"
	    do
		for INVERT_MATCH in "true" "false"
		do
		    for COUNT_HITS in "true" "false"
		    do
			for LINE_NUMBER in "true" "false"
			do
			    for FILE_NAMES in "true" "false"
			    do
				MAINFUNCTION+=`printf "\n  if(counter_unamb == %s && counter_unamb_rev == %s && verbose == %s && line_regexp == %s && invert_match == %s && count_hits == %s && print_line_numbers == %s && print_file_names == %s) {\n    return mainloop_u%s_ur%s_v%s_l%s_i%s_ch%s_ln%s_fn%s(grep_retval, bad_char_skip, result, regexp_text, filename, outfile);\n  }\n" $COUNTER_UNAMB $COUNTER_UNAMB_REV  $VERBOSE $LINE_REGEXP $INVERT_MATCH $COUNT_HITS $LINE_NUMBER $FILE_NAMES $COUNTER_UNAMB $COUNTER_UNAMB_REV  $VERBOSE $LINE_REGEXP $INVERT_MATCH $COUNT_HITS $LINE_NUMBER $FILE_NAMES`
				printf "int mainloop_u%s_ur%s_v%s_l%s_i%s_ch%s_ln%s_fn%s" $COUNTER_UNAMB $COUNTER_UNAMB_REV  $VERBOSE $LINE_REGEXP $INVERT_MATCH $COUNT_HITS $LINE_NUMBER $FILE_NAMES
				printf "(int grep_retval, size_t* bad_char_skip, regexp* result, char* regexp_text, char* filename, FILE * outfile){\n"
				if [ "${VERBOSE}" == "true" ] ; then
				    verbose_modifier="_verbose"
				    verbose_param_mod=", outfile"
				else
				    verbose_modifier=""
				    verbose_param_mod=""
				fi
				if [ $LINE_REGEXP == "false" ]; then
				    echo "  int occur_pos;"
				fi
				if [ $COUNT_HITS == "true" ]; then
				    echo "  int hit_counts = 0;"
				fi
				if [ $LINE_NUMBER == "true" ]; then
				    echo "  int line_number = 0;"
				fi
				cat <<EOF
  char* line;
  size_t line_len;
EOF
				if [ $COUNTER_UNAMB_REV == "true" ]; then
				    NEXTLINE_CMD='0, &line_len, result->min_word_length'
				else
				    NEXTLINE_CMD='result->min_word_length, &line_len, 0'
				fi
				echo "  while( ( ! get_file_finished() ) && ( line = nextline( $NEXTLINE_CMD ) ) ) {"
				if [ $LINE_NUMBER == "true" ]; then
				    echo "    line_number++;"
				fi
				if [ $LINE_REGEXP == "true" ]; then
				    echo "    bool word_rec;"
				    if [ $COUNTER_UNAMB == "true" ] ; then
					echo "    word_rec = recognize_whole_word${verbose_modifier}(line, line + line_len, result ${verbose_param_mod} );"
				    else
					echo "    word_rec = recognize_whole_word_rev${verbose_modifier}(line, line + line_len, result ${verbose_param_mod} );"
				    fi
				    printf "    if(word_rec != %s){\n" $INVERT_MATCH
				else
				    if [ $COUNTER_UNAMB_REV == "true" ]; then
					echo '    occur_pos = find_word(result, bad_char_skip, line, line_len);'
					STRING_MODIFY='line[line_len] = 0;'
				    else
					echo '    occur_pos = find_word_rev(result, bad_char_skip, line, line_len);'
					STRING_MODIFY='line += result->min_word_length;'
				    fi
				    printf "    if((occur_pos >= 0) != %s){\n" $INVERT_MATCH
				    echo "      $STRING_MODIFY"
				fi
				printHitLine
				echo "    }"
				echo "  }"
				if [ $COUNT_HITS == "true" ]; then
				    if [ $FILE_NAMES == "true" ]; then
					echo '  printf("%s:", filename);'
				    fi 
				    echo '  printf("%i\n", hit_counts);'
				fi
				cat <<EOF
  return grep_retval;
}

EOF
			    done
			done
		    done
		done
	    done
	done
    done
done

MAINFUNCTION+=`cat <<EOF 

  assert(false);
  return -1;
}

EOF`
printf "$MAINFUNCTION\n"