#include <asm-generic/errno-base.h>
#include <curses.h>
#include <errno.h>
#include <ncurses.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "idobata.h"
#include "mynet.h"

/* メッセージ送信者がサーバーの時にsockに渡す値 */
#define FROM_SERVER -1

/* Window */
static WINDOW *win_main, *win_sub;
/* Selectで監視するためのマスクと結果 */
fd_set mask, readfds;
/* selectで監視する最大値 */
static int Max_sd = 0;

static void init(int port_number, int *server_udp_sock, int *server_tcp_sock, int *client_tcp_sock);
static void recv_udp_packet(int udp_sock);
static void recv_msg_from_client();
static void delete_user(char *user_name, int sock);
static void register_username(member_t user, ido_packet_t *packet);
static void transfer_message(char *message, char *from_user_name, int from_sock);
static void setMax_sd(int num);

void idobata_server(int port_number) {
    int server_udp_sock;
    int server_tcp_sock;
    int client_tcp_sock;
    /* サーバー用の初期設定を行う*/
    init(port_number, &server_udp_sock, &server_tcp_sock, &client_tcp_sock);

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(0, &mask);
    FD_SET(server_udp_sock, &mask);
    FD_SET(server_tcp_sock, &mask);
    FD_SET(client_tcp_sock, &mask);

    /* メインループ */
    while (1) {
        /* 画面更新 */
        wrefresh(win_main);
        wrefresh(win_sub);
        /* 受信データの有無をチェック */
        readfds = mask;
        select(Max_sd + 1, &readfds, NULL, NULL, NULL);
        /* キーボードからの入力があった時 */
        if (FD_ISSET(0, &readfds)) {
            char p_buf[MSGBUF_SIZE];
            /* サブウィンドウでキーボードから文字列を入力する */
            /* 入力出来る文字は488バイトで、うち2バイトは改行とヌル文字にする */
            wgetnstr(win_sub, p_buf, MSGDATA_SIZE - 2);
            send_msg_from_keyboard(client_tcp_sock, p_buf);
            show_your_msg(win_main, p_buf);
            /* 入力用のプロンプトを表示する */
            wprintw(win_sub, "> ");
        }
        /* UDPパケットを受け取る時->サーバーを探しているクライアントがいた時 */
        else if (FD_ISSET(server_udp_sock, &readfds)) {
            recv_udp_packet(server_udp_sock);
        }
        // 新しいクライアントがコネクション張ろうとしている
        else if (FD_ISSET(server_tcp_sock, &readfds)) {
            /* クライアントの接続を受け付ける */
            int sock_accepted = Accept(server_tcp_sock, NULL, NULL);
            /* 名前は設定せずに登録 */
            add_user_to_list("", sock_accepted);
            /* クライアントとの通信を確認するため、ビットマスクをセット */
            FD_SET(sock_accepted, &mask);
            setMax_sd(sock_accepted);
        }
        /* サーバーからメッセージを受け取った時 */
        else if (FD_ISSET(client_tcp_sock, &readfds)) {
            char r_buf[MSGBUF_SIZE];
            int strsize = Recv(client_tcp_sock, r_buf, MSGBUF_SIZE - 1, 0);
            if (strsize == 0) {
                wprintw(win_main, "井戸端サーバーから切断しました。\nキー入力でクライアントを終了します。\n");
                wrefresh(win_main);
                close(client_tcp_sock);
                /* 何かのキー入力を待つ */
                wgetch(win_sub);
                return;
            }
            r_buf[strsize] = '\0';
            show_others_msg(win_main, r_buf);
        }
        /* サーバーがクライアントからメッセージを受け取った時 */
        else {
            recv_msg_from_client();
        }
    }
}

static void init(int port_number, int *server_udp_sock, int *server_tcp_sock, int *client_tcp_sock) {
    /* Windowを作成 */
    create_window(&win_main, &win_sub);
    wprintw(win_main, "サーバーモードで起動しました。\n");
    wprintw(win_sub, "> ");
    /* サーバーを起動する */
    /* UDPサーバーの初期化 */
    *server_udp_sock = init_udpserver((in_port_t)port_number);
    setMax_sd(*server_udp_sock);
    /* TCPサーバーの初期化 */
    *server_tcp_sock = init_tcpserver(port_number, 5);
    setMax_sd(*server_tcp_sock);
    /* 自分自身がサーバーなのでlocalhostを格納 */
    set_server_addr("localhost");
    /* 自分自身もサーバーに参加する */
    *client_tcp_sock = join_server(port_number);
    setMax_sd(*client_tcp_sock);
    wprintw(win_main, "サーバーに参加しました。\n");
}

/* UDPパケットを受け取る時->サーバーを探しているクライアントがいた時 */
/* HEREパケットを返す */
static void recv_udp_packet(int udp_sock) {
    struct sockaddr_in from_adrs;
    unsigned int from_len;

    char r_buf[MSGBUF_SIZE];
    char s_buf[MSGBUF_SIZE];
    int strsize;
    ido_packet_t *packet;

    // UDPでパケットを受信したらサーバーを探しているクライアントがいるので
    /* 文字列をクライアントから受信する */
    from_len = sizeof(from_adrs);
    strsize = Recvfrom(udp_sock, r_buf, MSGBUF_SIZE, 0, (struct sockaddr *)&from_adrs, &from_len);

    packet = (ido_packet_t *)r_buf; /* packetがバッファの先頭を指すようにする */
    /* HELLOパケット以外は受け取らない */
    if (analyze_header(packet->header) != HELLO) {
        return;
    }
    /* 文字列をクライアントに送信する */
    /* HEREパケットを作成 */
    create_packet(s_buf, HERE, "");
    strsize = strlen(s_buf);

    /* HEREパケットをを送信者に送信する */
    Sendto(udp_sock, s_buf, strsize, 0, (struct sockaddr *)&from_adrs, sizeof(from_adrs));
}

static void recv_msg_from_client() {
    char r_buf[MSGBUF_SIZE];
    int strsize;
    // 特定のユーザからメッセージが届いている
    member_t current = get_head_from_list();
    // 全メンバーのsockを確認する
    while (current != NULL) {
        int client_sock = current->sock;
        ido_packet_t *packet;
        /* このユーザからはメッセージが届いていないのでスキップ */
        if (!FD_ISSET(client_sock, &readfds)) {
            current = current->next;
            continue;
        }
        // このユーザから受信する
        strsize = Recv(client_sock, r_buf, MSGBUF_SIZE - 1, 0);
        r_buf[strsize] = '\0';
        packet = (ido_packet_t *)r_buf;

        // 切断された時
        if (strsize == 0) {
            delete_user(current->username, current->sock);
            current = current->next;
            continue;
        }

        // 名前が未設定->まだ参加していない
        if (strlen(current->username) == 0) {
            /* 名前を登録する */
            register_username(current, packet);
            current = current->next;
            continue;
        }

        // ヘッダで分岐させる
        // ここに来るメッセージはPOSTかQUITしか無い
        switch (analyze_header(packet->header)) {
            case POST:
                // メッセージを転送する
                transfer_message(packet->data, current->username, current->sock);
                break;
            case QUIT:
                // ユーザの登録情報を削除する
                delete_user(current->username, current->sock);
                break;
            default:
                break;
        }
        current = current->next;
    }
}

/* ユーザ名前を登録する */
static void register_username(member_t user, ido_packet_t *packet) {
    char message[MSGDATA_SIZE];
    /* JOINパケットだったら登録する */
    if (analyze_header(packet->header) == JOIN) {
        // 名前を登録する
        snprintf(user->username, USERNAME_LEN, "%s", packet->data);
        // ユーザが参加したことを知らせるメッセージを送信する
        snprintf(message, MSGDATA_SIZE, "%sが参加しました\n", user->username);
        transfer_message(message, "Server", FROM_SERVER);
    }
}

/* ユーザの登録情報を削除する */
static void delete_user(char *user_name, int sock) {
    char s_buf[MSGDATA_SIZE];
    snprintf(s_buf, MSGDATA_SIZE, "%sがサーバーから切断しました。\n", user_name);
    transfer_message(s_buf, "Server", -1);
    /* selectの対象外にする */
    FD_CLR(sock, &mask);
    /* コネクションを終了する */
    close(sock);
    /* ユーザを削除する */
    delete_user_from_list(sock);
}

/* メッセージを転送する */
/* 誰からのメッセージかをfrom_user_nameに格納 */
/* 送信者以外に転送するので、送信者をfrom_sockで識別する */
static void transfer_message(char *message, char *from_user_name, int from_sock) {
    char s_buf[MSGBUF_SIZE];
    char name_p_message[MSGDATA_SIZE];
    // ユーザ名を付加したメッセージを生成
    snprintf(name_p_message, MSGDATA_SIZE, "[%s]%s", from_user_name, message);

    // MESSAGEパケットを作成する
    create_packet(s_buf, MESSAGE, name_p_message);

    member_t current = get_head_from_list();
    while (current != NULL) {
        // メッセージを送信者以外に送信する
        if (current->sock != from_sock) {
            int ret = send(current->sock, s_buf, MSGBUF_SIZE, MSG_NOSIGNAL);
            if (ret == -1 && errno == EPIPE) {
                // 通信相手が切断していた時
                delete_user(current->username, current->sock);
            }
        }
        current = current->next;
    }
}

/* Max_sdの値は常に最大値を取るようにする*/
static void setMax_sd(int num) {
    if (num > Max_sd) {
        Max_sd = num;
    }
}
