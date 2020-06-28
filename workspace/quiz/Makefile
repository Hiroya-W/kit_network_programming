#
# Makefile for echo_fork echo_thread
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g

quiz: quiz.o quiz_client.o quiz_common.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

# quiz_client.o: quiz_client.c
# 	${CC} ${CFLAGS} -o $@ $^ ${MYLIB} quiz.h
# 
# quiz_common.o: quiz_common.c
# 	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o quiz *~
