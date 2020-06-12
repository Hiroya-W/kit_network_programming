#
# Makefile for echo_fork echo_thread
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet -lpthread
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g

all: echo_fork echo_thread

echo_fork: echo_fork.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

echo_thread: echo_thread.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echo_fork echo_thread *~
