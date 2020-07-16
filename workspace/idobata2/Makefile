#
# Makefile for idobata
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet -lncursesw
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g
# OBJS = chat.o chat_client.o chat_server.o chat_common.o
OBJS = idobata.o idobata_common.o idobata_client.o idobata_server.o window.o packet.o linear_lists.o

all: idobata

idobata: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

${OBJS}: idobata.h

clean:
	${RM} *.o idobata *~
