/*
 * chat_server.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "chat.h"
#include "mynet.h"

#define NAMELENGTH 20   /* ログイン名の長さ制限 */
#define RECV_BUFLEN 500 /* 通信バッファサイズ */
#define SEND_BUFLEN 600 /* 通信バッファサイズ */

/* 各クライアントのユーザ情報を格納する構造体の定義 */
typedef struct {
    int sock;
    char name[NAMELENGTH];
} client_info;

static int Max_client;      /* クライアントの数 */
static client_info *Client; /* クライアントの情報 */
static int Max_sd = 0;      /* ディスクリプタ最大値 */
static int Cnt_client = 0;  /* 現在のクライアントの接続数 */
static char SET_NAME_MESSAGE[] = "Input your name: ";

static void init_client(int n_client);
static int client_join(int client_id, int sock_listen);
static void client_exit(int client_id);
static void set_client_name(char *client_name, int client_id);
static void send_chat_message(char *message, int from_client);
static void handle_recv_data(int client_id, char *recv_buf, int strsize);
static void setMax_sd(int num);
static char *chop_nl(char *s);

void chat_server(int port_number, int n_client) {
    fd_set mask, readfds;
    int client_id;

    /* サーバの初期化 */
    const int SOCK_LISTEN = init_tcpserver(port_number, 5);
    /* selectで監視する値は現在、sock_listenまでである */
    setMax_sd(SOCK_LISTEN);

    /* クライアント情報の初期化 */
    init_client(n_client);

    /* selectで接続があったか確認したい */
    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(SOCK_LISTEN, &mask);

    while (1) {
        readfds = mask;
        select(Max_sd + 1, &readfds, NULL, NULL, NULL);

        /* 接続上限を超えて接続があった場合はメッセージを返して通信を終了する */
        if (Cnt_client >= Max_client && FD_ISSET(SOCK_LISTEN, &readfds)) {
            char FULL_MESSAGE[] = "This server is full.\n";
            /* クライアントの接続を受け付ける */
            const int SOCK_ACCEPTED = Accept(SOCK_LISTEN, NULL, NULL);
            /* サーバーが満員であることを通知して切断する */
            printf("%s sock_accepted[%d] disconnected.\n", FULL_MESSAGE, SOCK_ACCEPTED);
            Send(SOCK_ACCEPTED, FULL_MESSAGE, strlen(FULL_MESSAGE), 0);
            close(SOCK_ACCEPTED);
        }

        for (client_id = 0; client_id < Max_client; client_id++) {
            const int CLIENT_SOCK = Client[client_id].sock;
            /* サーバが満員ではない時、新たに接続するクライアントがいるか確認してみる */
            if ((CLIENT_SOCK == -1) && FD_ISSET(SOCK_LISTEN, &readfds)) {
                /* クライアントの接続を受け付ける */
                const int SOCK_ACCEPTED = client_join(client_id, SOCK_LISTEN);
                /* クライアントとの通信を確認するため、ビットマスクをセット */
                FD_SET(SOCK_ACCEPTED, &mask);
                /* 状態を更新 */
                readfds = mask;
                select(Max_sd + 1, &readfds, NULL, NULL, NULL);
            }
            /* クライアントが接続していなかったらスキップ */
            if (CLIENT_SOCK == -1) {
                continue;
            }
            /* 接続しているクライアントからのメッセージに対しての処理 */
            if (FD_ISSET(CLIENT_SOCK, &readfds)) {
                char recv_buf[RECV_BUFLEN]; /* 通信用バッファ */
                /* クライアントからデータが届いているので受信する */
                int strsize = Recv(Client[client_id].sock, recv_buf, RECV_BUFLEN - 1, 0);
                recv_buf[strsize] = '\0';

                /* メッセージ内容に合わせて処理する */
                handle_recv_data(client_id, recv_buf, strsize);
            }
        }
    }
    close(SOCK_LISTEN);
}

/* クライアント情報の初期化 */
static void init_client(const int N_CLIENT) {
    int client_id;

    /* クライアントの最大接続数をセット */
    Max_client = N_CLIENT;
    /* クライアント情報の保存用構造体の初期化 */
    if ((Client = (client_info *)malloc(Max_client * sizeof(client_info))) == NULL) {
        exit_errmesg("malloc()");
    }
    /* 接続されていないユーザはsock==-1とする */
    /* ユーザ名は""が入ってる */
    for (client_id = 0; client_id < Max_client; client_id++) {
        Client[client_id].sock = -1;
        strncpy(Client[client_id].name, "", NAMELENGTH);
    }
}

/* クライアントの参加処理 */
static int client_join(int client_id, int sock_listen) {
    int sock_accepted;
    /* クライアントの接続を受け付ける */
    sock_accepted = Accept(sock_listen, NULL, NULL);

    /* sockの最大値を更新する */
    setMax_sd(sock_accepted);

    /* ユーザ情報を保存 */
    printf("Client[%d] sock_accepted[%d] connected.\n", client_id, sock_accepted);
    Client[client_id].sock = sock_accepted;

    /* 名前登録用のプロンプトを送信 */
    Send(sock_accepted, SET_NAME_MESSAGE, strlen(SET_NAME_MESSAGE), 0);

    /* 現在の接続数をカウント */
    Cnt_client++;
    /* selectによる監視のためにsock_acceptedを返す*/
    return sock_accepted;
}

/* クライアントの退出処理 */
static void client_exit(int client_id) {
    char send_message[SEND_BUFLEN];
    char client_name[NAMELENGTH];
    int client_sock = Client[client_id].sock;

    snprintf(client_name, NAMELENGTH, "%s", Client[client_id].name);

    /* 通信を切断する */
    close(client_sock);
    /* 新しい接続を受け付けるようにクライアント情報を初期化する */
    Client[client_id].sock = -1;
    strncpy(Client[client_id].name, "", NAMELENGTH);
    Cnt_client--;
    /* 今いるクライアントに対して退出の通知を行う */
    printf("%s Client[%d] sock_accepted[%d] disconnected.\n", client_name, client_id, client_sock);
    snprintf(send_message, SEND_BUFLEN, "%s disconnected.\n", client_name);
    send_chat_message(send_message, -1);
}

/* クライアントの名前をセットする */
static void set_client_name(char *client_name, int client_id) {
    /* 名前をセットする */
    /* 改行だけは登録しない */
    if (strcmp(client_name, "\n") == 0) {
        /* もう一度名前登録用のプロンプトを送信 */
        Send(Client[client_id].sock, SET_NAME_MESSAGE, strlen(SET_NAME_MESSAGE), 0);
    } else {
        char send_message[SEND_BUFLEN];
        /* クライアント名から改行を除去 */
        chop_nl(client_name);
        /* クライアント名を登録 */
        strncpy(Client[client_id].name, client_name, NAMELENGTH);
        /* クライアント参加の通知を行う */
        snprintf(send_message, SEND_BUFLEN, "%s connected.\n", client_name);
        printf("%s", send_message);
        send_chat_message(send_message, -1);
    }
}

/* 送信者以外に対してメッセージを送信する。 */
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

    for (client_id = 0; client_id < Max_client; client_id++) {
        /* 通信が確立している相手かつ、送信者以外に送信する */
        int sock = Client[client_id].sock;
        if (sock != -1 && client_id != from_client) {
            Send(sock, send_message, strlen(send_message), 0);
        }
    }
}

/* メッセージ内容に合わせて処理する */
static void handle_recv_data(int client_id, char *recv_buf, int strsize) {
    /* クライアントが切断したか確認する */
    /* 切断された時はRecvから0が返ってくる */
    if (strsize == 0) {
        client_exit(client_id);
    }
    /* メッセージを受信した時 */
    /* 名前がセットされていない時、そのメッセージは名前登録として扱う */
    else if (strlen(Client[client_id].name) == 0) {
        set_client_name(recv_buf, client_id);
    }
    /* 名前がセットされていた時、それはチャットメッセージとして扱う */
    else {
        /* 送信者以外に対してメッセージを送信する */
        send_chat_message(recv_buf, client_id);
    }
}

/* Max_sdの値は常に最大値を取るようにする*/
static void setMax_sd(int num) {
    if (num > Max_sd) {
        Max_sd = num;
    }
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
