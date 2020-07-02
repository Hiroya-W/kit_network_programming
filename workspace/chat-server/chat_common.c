/*
 * chat_common.c
 */

#include "mynet.h"

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int r;
    if ((r = accept(s, addr, addrlen)) == -1) {
        exit_errmesg("accept()");
    }
    return (r);
}

int Send(int s, void *buf, size_t len, int flags) {
    int r;
    if ((r = send(s, buf, len, flags)) == -1) {
        exit_errmesg("send()");
    }
    return (r);
}

int Recv(int s, void *buf, size_t len, int flags) {
    int r;
    if ((r = recv(s, buf, len, flags)) == -1) {
        exit_errmesg("recv()");
    }
    return (r);
}

/* 日本語の出現回数をカウントする関数 */
int cnt_jp(char *str) {
    /* UTF8で日本語は3バイト */
    /* 2byte目移行なら (*str & 0xC0) == 0x80 となる */
    /* 結果的に日本語のときにだけcountが2増える */
    int count = 0;
    while (*str != '\0') {
        if ((*str & 0xC0) == 0x80) {
            count++;
        }
        str++;
    }
    return count / 2;
}
