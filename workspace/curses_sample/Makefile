#
# Makefile for tcp_echo
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet -lncurses
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall

curses: curses.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o curses *~
