#
# Makefile for quiz_server and quiz_client
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g
OBJS = quiz.o quiz_server.o quiz_client.o quiz_util.o question.o quiz_common.o

quiz: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o quiz *~
