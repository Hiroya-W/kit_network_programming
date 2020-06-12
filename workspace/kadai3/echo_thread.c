/*
  echo_server3th.c (Thread版)
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mynet.h"

#define BUFSIZE 50 /* バッファサイズ */

void *echo_thread(void *arg);

int main(int argc, char *argv[]) {
    int thread_limit = 10;
    int port_number = 50000;
    int sock_listen;
    int *tharg;
    pthread_t tid;

    extern char *optarg;
    extern int optind, opterr, optopt;

    /* オプション文字列の取得 */
    opterr = 0;
    int c = 0;
    while (1) {
        c = getopt(argc, argv, "p:r:h");
        if (c == -1) break;

        switch (c) {
            case 'p': /* ポート番号の指定 */
                port_number = atoi(optarg);
                break;
            case 'r':
                thread_limit = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optopt);
                exit(EXIT_FAILURE);
                break;
            case 'h':
                fprintf(stderr, "Usage: %s [-p port_number] [-r thread_limit]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    fprintf(stdout, "終了する時はEnterキーを押すか、Ctrl+Cしてください\n");
    fprintf(stdout, "Server Port: %d\n", port_number);
    fprintf(stdout, "Thread Limit: %d\n\n", thread_limit);

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    int i;
    for (i = 0; i < thread_limit; i++) {
        /* スレッド関数の引数を用意する */
        if ((tharg = (int *)malloc(sizeof(int))) == NULL) {
            exit_errmesg("malloc()");
        }
        *tharg = sock_listen;
        /* スレッドを生成する */
        if (pthread_create(&tid, NULL, echo_thread, (void *)tharg) != 0) {
            exit_errmesg("pthread_create()");
        }
    }

    /* 入力待ちで一時停止させる */
    getchar();
    return 0;
}

/* スレッドの本体 */
void *echo_thread(void *arg) {
    int sock_listen;
    int sock_accepted;
    char buf[BUFSIZE];
    int strsize;

    sock_listen = *((int *)arg);
    free(arg); /* 引数用のメモリを開放 */

    pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */

    fprintf(stdout, "Thread is created. thread_id[%ld]\n", pthread_self());
    while (1) {
        /* クライアントの接続を受け付ける */
        sock_accepted = accept(sock_listen, NULL, NULL);
        do {
            /* 文字列をクライアントから受信する */
            if ((strsize = recv(sock_accepted, buf, BUFSIZE, 0)) == -1) {
                exit_errmesg("recv()");
            }

            /* 文字列をクライアントに送信する */
            if (send(sock_accepted, buf, strsize, 0) == -1) {
                exit_errmesg("send()");
            }
        } while (buf[strsize - 1] != '\n'); /* 改行コードを受信するまで繰り返す */

        close(sock_accepted); /* ソケットを閉じる */
    }

    return (NULL);
}
