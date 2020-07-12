#
# Makefile for udp_echo
#
CC = gcc
MYLIBDIR =../mynet
MYLIB =-lmynet
CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall

all: udp_client udp_server udp_client1 udp_server1

udp_client: udp_client.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

udp_server: udp_server.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

udp_client1: udp_client1.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

udp_server1: udp_server1.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o udp_client *~
