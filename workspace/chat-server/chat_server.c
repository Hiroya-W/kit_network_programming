/*
 * quiz_server.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "chat.h"
#include "mynet.h"

#define NAMELENGTH 20 /* ログイン名の長さ制限 */
#define BUFLEN 500    /* 通信バッファサイズ */

/* 各クライアントのユーザ情報を格納する構造体の定義 */
typedef struct {
    int sock;
    char name[NAMELENGTH];
} client_info;

/* プライベート変数 */
static int N_client;        /* クライアントの数 */
static client_info *Client; /* クライアントの情報 */
static int Max_sd = 0;      /* ディスクリプタ最大値 */
static char Buf[BUFLEN];    /* 通信用バッファ */
int cnt_client = 0;

void setMax_sd(int num);

void chat_server(int port_number, int n_client) {
    /* select用のビットマスク */
    fd_set mask, readfds;
    /* クライアントの最大接続数をセット */
    N_client = n_client;

    int client_id;

    int sock_listen;
    int sock_accepted;

    int strsize;

    static char full_message[] = "This server is full.\n";

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);
    /* selectで監視する値は現在、sock_listenまでである */
    setMax_sd(sock_listen);

    /* クライアント情報の保存用構造体の初期化 */
    if ((Client = (client_info *)malloc(N_client * sizeof(client_info))) == NULL) {
        exit_errmesg("malloc()");
    }
    /* 接続されていないユーザはsock==-1とする */
    for (client_id = 0; client_id < N_client; client_id++) {
        Client[client_id].sock = -1;
    }

    /* selectで接続があったか確認したい */
    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(sock_listen, &mask);

    while (1) {
        readfds = mask;
        select(Max_sd + 1, &readfds, NULL, NULL, NULL);

        /* 接続上限を超えて接続があった場合はメッセージを返して通信を終了する */
        if (cnt_client >= N_client && FD_ISSET(sock_listen, &readfds)) {
            /* クライアントの接続を受け付ける */
            sock_accepted = Accept(sock_listen, NULL, NULL);
            printf("%s sock_accepted[%d] disconnected.\n", full_message, sock_accepted);
            Send(sock_accepted, full_message, strlen(full_message), 0);
            close(sock_accepted);
        }

        for (client_id = 0; client_id < N_client; client_id++) {
            /* クライアントの接続 */
            /* 接続されていないユーザがいたら、接続するか確認してみる */
            if ((Client[client_id].sock == -1) && FD_ISSET(sock_listen, &readfds)) {
                /* クライアントの接続を受け付ける */
                sock_accepted = Accept(sock_listen, NULL, NULL);
                /* sockの最大値を更新する */
                setMax_sd(sock_accepted);
                /* クライアントの通信を確認するためのビットマスクをセット */
                FD_SET(sock_accepted, &mask);
                /* ユーザ情報を保存 */
                printf("Client[%d] sock_accepted[%d] connected.\n", cnt_client, sock_accepted);
                Client[client_id].sock = sock_accepted;
                cnt_client++;
            }
            /* 接続されていないクライアントだったらスキップ */
            if (Client[client_id].sock == -1) {
                continue;
            }
            /* 接続されている時 */
            /* クライアントの切断があったか確認する */
            else {
                readfds = mask;
                select(Max_sd + 1, &readfds, NULL, NULL, NULL);
                /* TODO 受信したときの処理に統合させる */
                if (FD_ISSET(Client[client_id].sock, &readfds)) {
                    strsize = Recv(Client[client_id].sock, Buf, BUFLEN - 1, 0);
                    Buf[strsize] = '\0';

                    /* 切断された時はRecvから0が返ってくる */
                    if (strsize == 0) {
                        printf("Client[%d] sock_accepted[%d] disconnected.\n", client_id, sock_accepted);
                        close(Client[client_id].sock);
                        /* 再び接続を受け付けるようにする */
                        Client[client_id].sock = -1;
                        cnt_client--;
                        fflush(stdout);
                    } else {
                        printf("%s\n", Buf);
                    }
                }
            }
        }
    }
    close(sock_listen);

    /* メインループ */
    // question_loop();
}

/* Max_sdを更新する時はこの関数を使うようにする */
void setMax_sd(int num) {
    Max_sd = Max_sd < num ? num : Max_sd;
}
