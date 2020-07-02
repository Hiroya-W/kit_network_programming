#
# Makefile for chat-server
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet -lncursesw
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g
OBJS = chat.o chat_client.o chat_server.o chat_common.o

chat: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o chat *~
