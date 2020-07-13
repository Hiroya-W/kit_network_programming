#
# Makefile for idobata
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet -lncursesw
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g
# OBJS = chat.o chat_client.o chat_server.o chat_common.o
OBJS = idobata.o packet.o linear_lists.o

all: idobata

idobata: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o idobata linear_lists *~
