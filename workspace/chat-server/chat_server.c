/*
 * quiz_server.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "chat.h"
#include "mynet.h"

#define NAMELENGTH 20   /* ログイン名の長さ制限 */
#define BUFLEN 500      /* 通信バッファサイズ */
#define SEND_BUFLEN 600 /* 通信バッファサイズ */

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
static char *chop_nl(char *s);
static void send_chat_message(char *message, int from_client);

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
    static char set_name_message[] = "Input your name: ";

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);
    /* selectで監視する値は現在、sock_listenまでである */
    setMax_sd(sock_listen);

    /* クライアント情報の保存用構造体の初期化 */
    if ((Client = (client_info *)malloc(N_client * sizeof(client_info))) == NULL) {
        exit_errmesg("malloc()");
    }
    /* 接続されていないユーザはsock==-1とする */
    /* ユーザ名は""が入ってる */
    for (client_id = 0; client_id < N_client; client_id++) {
        Client[client_id].sock = -1;
        strncpy(Client[client_id].name, "", NAMELENGTH);
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
                printf("Client[%d] sock_accepted[%d] connected.\n", client_id, sock_accepted);
                Client[client_id].sock = sock_accepted;

                /* 名前登録用のプロンプトを送信 */
                Send(sock_accepted, set_name_message, strlen(set_name_message), 0);

                /* 現在の接続数をカウント */
                cnt_client++;

                /* 状態を更新 */
                readfds = mask;
                select(Max_sd + 1, &readfds, NULL, NULL, NULL);
            }
            /* 接続されている時 */
            else if (Client[client_id].sock != -1) {
                if (FD_ISSET(Client[client_id].sock, &readfds)) {
                    strsize = Recv(Client[client_id].sock, Buf, BUFLEN - 1, 0);
                    Buf[strsize] = '\0';

                    /* クライアントの切断があったか確認する */
                    /* 切断された時はRecvから0が返ってくる */
                    if (strsize == 0) {
                        char send_message[SEND_BUFLEN];
                        printf("%s Client[%d] sock_accepted[%d] disconnected.\n", Client[client_id].name, client_id, Client[client_id].sock);
                        snprintf(send_message, SEND_BUFLEN, "%s disconnected.\n", Client[client_id].name);
                        close(Client[client_id].sock);
                        /* 再び接続を受け付けるようにする */
                        Client[client_id].sock = -1;
                        strncpy(Client[client_id].name, "", NAMELENGTH);
                        cnt_client--;
                        /* ユーザ退出の通知を行う */
                        send_chat_message(send_message, -1);
                    }
                    /* 文字列を受信した時 */
                    else {
                        /* 名前がセットされていない時 */
                        if (strlen(Client[client_id].name) == 0) {
                            /* 名前をセットさせる */
                            /* 改行だけはダメ */
                            if (strcmp(Buf, "\n") == 0) {
                                /* もう一度名前登録用のプロンプトを送信 */
                                Send(Client[client_id].sock, set_name_message, strlen(set_name_message), 0);
                            } else {
                                char send_message[SEND_BUFLEN];
                                /* ユーザ名から改行を除去 */
                                chop_nl(Buf);
                                /* ユーザ名を登録 */
                                strncpy(Client[client_id].name, Buf, NAMELENGTH);
                                /* ユーザ参加の通知を行う */
                                snprintf(send_message, SEND_BUFLEN, "%s connected.\n", Buf);
                                printf("%s", send_message);
                                send_chat_message(send_message, -1);
                            }
                        } else {
                            /* 全員に対してメッセージを送信する */
                            send_chat_message(Buf, client_id);
                        }
                    }
                }
            }
        }
    }
    close(sock_listen);
}

/* 全員に対してメッセージを送信する。 */
static void send_chat_message(char *message, int from_client) {
    int client_id;
    char send_message[SEND_BUFLEN];
    char from_client_name[NAMELENGTH];

    /* -1の時はサーバーからのメッセージ扱いにする */
    if (from_client < 0) {
        snprintf(from_client_name, NAMELENGTH, "Server");
    } else {
        snprintf(from_client_name, NAMELENGTH, "%s", Client[from_client].name);
    }

    snprintf(send_message, SEND_BUFLEN, "[%s]: %s\n", from_client_name, message);
    printf("%s", send_message);

    for (client_id = 0; client_id < N_client; client_id++) {
        /* 通信が確立している相手かチェックする */
        int sock = Client[client_id].sock;
        if (sock != -1) {
            Send(sock, send_message, strlen(send_message), 0);
        }
    }
}

/* Max_sdを更新する時はこの関数を使うようにする */
void setMax_sd(int num) {
    Max_sd = Max_sd < num ? num : Max_sd;
}

/* 改行除去 */
static char *chop_nl(char *s) {
    int len;
    len = strlen(s);
    if (s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
    return (s);
}
