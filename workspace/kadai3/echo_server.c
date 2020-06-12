#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mynet.h"

#define BUFSIZE 50 /* バッファサイズ */

enum Mode {
    Fork,
    Thread,
};

void *echo_thread(void *arg);

int main(int argc, char *argv[]) {
    int mode = Fork;
    int limit = 10;
    int port_number = 50000;
    int sock_listen, sock_accepted;
    pid_t child;
    char buf[BUFSIZE];
    int strsize;

    int *tharg;
    pthread_t tid;

    extern char *optarg;
    extern int optind, opterr, optopt;

    /* オプション文字列の取得 */
    opterr = 0;
    int c = 0;
    while (1) {
        c = getopt(argc, argv, "p:ftl:h");
        if (c == -1) break;

        switch (c) {
            case 'p': /* ポート番号の指定 */
                port_number = atoi(optarg);
                break;
            case 'f':
                mode = Fork;
                break;
            case 't':
                mode = Thread;
                break;
            case 'l':
                limit = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optopt);
                exit(EXIT_FAILURE);
                break;
            case 'h':
                fprintf(stderr, "USAGE:\n");
                fprintf(stderr, "    %s [OPTIONS]\n\n", argv[0]);
                fprintf(stderr, "OPTIONS:\n");
                fprintf(stderr, "    -p, <number> \t Set port number [default: 50000]\n");
                fprintf(stderr, "    -f, \t\t Create process by using fork system call\n");
                fprintf(stderr, "    -t, \t\t Create thread instead of fork\n");
                fprintf(stderr, "    -l, <number> \t Set limit of the number of processes or threads [default: 10]\n");
                exit(EXIT_FAILURE);
                break;
        }
    }

    fprintf(stdout, "終了する時はEnterキーを押すか、Ctrl+Cしてください\n");
    fprintf(stdout, "Server Port: %d\n", port_number);
    fprintf(stdout, "Limit: %d\n", limit);

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    int i;
    switch (mode) {
        case Fork:
            fprintf(stdout, "Mode: Fork\n\n");

            // 親は子プロセスを作るだけでクライアントと接続はしない
            // 子をprcs_limitだけ作る
            printf("Parent ppid[%d],pid[%d]\n", getppid(), getpid());
            for (i = 0; i < limit; i++) {
                if ((child = fork()) == 0) {
                    /* child process */
                    printf("Client is accepted.ppid[%d],pid[%d]\n", getppid(), getpid());
                    while (1) {
                        /* クライアントの接続を受け付ける */
                        sock_accepted = accept(sock_listen, NULL, NULL);
                        printf("Connected.pid[%d]\n", getpid());

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

                        close(sock_accepted);
                    }
                    break;
                } else if (child > 0) {
                    /* parent's process */
                    /* 何もしない */
                } else {
                    /* fork()に失敗 */
                    close(sock_listen);
                    exit_errmesg("fork()");
                }
            }
            break;
        case Thread:
            fprintf(stdout, "Mode: Thread\n\n");

            for (i = 0; i < limit; i++) {
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
            break;
    }

    getchar();
    /* ここにたどり着くのは親だけ
     * 特にfork modeの時、
     * 親が属するプロセスグループ全体にSIGINTを送信することで、子プロセスを終了させる
     */
    kill(0, SIGINT);
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
