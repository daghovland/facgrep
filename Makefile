CC = gcc
#CFLAGS = -Wall -g -floop-optimize -ftree-loop-ivcanon -ftree-loop-linear -ftree-loop-im  -pipe --std=c99
CFLAGS = -Wall -g   -pipe --std=c99
#CFLAGS = -Wall -O3 -floop-optimize -ftree-loop-ivcanon -ftree-loop-linear -ftree-loop-im  -pipe --std=c99
#CFLAGS = -Wall -pipe --std=c99
LDFLAGS_POST = -ll -lfl 
LDFLAGS = 
PARSER = parser
OBJS = $(PARSER).o regexp.o init.o fac_gen.o boyermoore.o mainloop.o  filereader.o


all: grep

grep: $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDFLAGS_POST) 


$(PARSER).c: lexer grep.y
	bison -d -o $(PARSER).c grep.y

lexer: grep.lex
	flex -8 grep.lex



clean:
	rm -f lex.yy.c y.tab.c $(PARSER).c *.o grep.tab.h grep mainloop.c findword.c fac_gen.c follow_prec.c


mainloop.c: create_mainloop.sh
	./create_mainloop.sh  > mainloop.c

boyermoore.c: findword.c

findword.c: create_findword.sh 
	./create_findword.sh  > findword.c

fac_gen.c: create_fac.sh
	./create_fac.sh  > fac_gen.c

regexp.c: follow_prec.c

follow_prec.c: create_follow_prec.sh
	./create_follow_prec.sh  > follow_prec.c
