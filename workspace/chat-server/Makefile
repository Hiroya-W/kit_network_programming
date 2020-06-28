#
# Makefile for chat-server
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet 
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g
OBJS = chat.o

all: chat

chat: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o chat *~
