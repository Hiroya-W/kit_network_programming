#
# Makefile for tcp_thread
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet -lpthread
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g

all: echo_thread

echo_thread: echo_thread.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echo_thread *~
