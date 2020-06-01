#
# Makefile for tcp_echo
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet -lcrypt
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g

all: echo_server2

echo_server2: echo_server2.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echo_server2 *~
