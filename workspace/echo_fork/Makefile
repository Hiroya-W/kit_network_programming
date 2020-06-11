#
# Makefile for tcp_fork
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet -lcrypt
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g

all: echo_fork

echo_fork: echo_fork.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echo_fork *~
