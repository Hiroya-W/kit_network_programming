#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mynet.h"

#define BUFSIZE 50 /* バッファサイズ */

int main(int argc, char *argv[]) {
    int prcs_limit = 10;
    int port_number = 50000;
    int sock_listen, sock_accepted;
    pid_t child;
    char buf[BUFSIZE];
    int strsize;

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
                prcs_limit = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optopt);
                exit(EXIT_FAILURE);
                break;
            case 'h':
                fprintf(stderr, "Usage: %s [-p port_number] [-r process_limit]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    fprintf(stdout, "終了する時はEnterキーを押すか、Ctrl+Cしてください\n");
    fprintf(stdout, "Server Port: %d\n", port_number);
    fprintf(stdout, "Process Limit: %d\n\n", prcs_limit);

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    // 親は子プロセスを作るだけでクライアントと接続はしない
    // 子をprcs_limitだけ作る
    printf("Parent ppid[%d],pid[%d]\n", getppid(), getpid());
    int i;
    for (i = 0; i < prcs_limit; i++) {
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

    getchar();
    /* ここにたどり着くのは親だけ
     * 親が属するプロセスグループ全体にSIGINTを送信することで、子プロセスを終了させる
     */
    kill(0, SIGINT);
    return 0;
}
