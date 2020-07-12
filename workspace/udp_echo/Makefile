#
# Makefile for udp_echo
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall

all: udp_client udp_server

udp_client: udp_client.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

udp_server: udp_server.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o udp_client *~
