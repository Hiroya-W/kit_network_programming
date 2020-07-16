#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#include "idobata.h"
#include "mynet.h"

static WINDOW *win_main, *win_sub;
static void init(int port_number, int *sock);

void idobata_client(int port_number) {
    int sock;
    fd_set mask, readfds;
    init(port_number, &sock);

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(0, &mask);
    FD_SET(sock, &mask);

    /* メインループ */
    while (1) {
        /* 画面更新 */
        wrefresh(win_main);
        wrefresh(win_sub);
        /* 受信データの有無をチェック */
        readfds = mask;
        select(sock + 1, &readfds, NULL, NULL, NULL);

        /* キーボードからの入力があった時 */
        if (FD_ISSET(0, &readfds)) {
            char p_buf[MSGBUF_SIZE];
            /* サブウィンドウでキーボードから文字列を入力する */
            /* 入力出来る文字は488バイトで、うち2バイトは改行とヌル文字にする */
            wgetnstr(win_sub, p_buf, MSGDATA_SIZE - 2);
            send_msg_from_keyboard(sock, p_buf);
            show_your_msg(win_main, p_buf);
            /* 入力用のプロンプトを表示する */
            wprintw(win_sub, "> ");
        }
        /* サーバーからメッセージを受け取った時 */
        else if (FD_ISSET(sock, &readfds)) {
            char r_buf[MSGBUF_SIZE];
            int strsize = Recv(sock, r_buf, MSGBUF_SIZE - 1, 0);
            if (strsize == 0) {
                wprintw(win_main, "井戸端サーバーから切断しました。\nキー入力でクライアントを終了します。\n");
                wrefresh(win_main);
                close(sock);
                /* 何かのキー入力を待つ */
                wgetch(win_sub);
                return;
            }
            r_buf[strsize] = '\0';
            show_others_msg(win_main, r_buf);
        }
    }
}

static void init(int port_number, int *sock) {
    /* Windowを作成 */
    create_window(&win_main, &win_sub);
    wprintw(win_main, "クライアントモードで起動しました。\n");
    /* クライアント用の初期設定を行う*/
    *sock = join_server(port_number);
    wprintw(win_main, "サーバーに参加しました。\n");
    wprintw(win_sub, "> ");
}

/* サーバーに参加する */
int join_server(int port_number) {
    char server_addr[20];
    char user_name[USERNAME_LEN];
    char s_buf[MSGBUF_SIZE];
    int sock;
    int strsize;
    /* 格納されているサーバーアドレスを取得する */
    get_server_addr(server_addr);
    /* 格納されているユーザ名を取得する */
    get_user_name(user_name);
    /* サーバーに接続する */
    sock = init_tcpclient(server_addr, port_number);
    /* JOINパケットを送信する */
    create_packet(s_buf, JOIN, user_name);
    strsize = strlen(s_buf);
    Send(sock, s_buf, strsize, 0);
    return sock;
}

/* キーボード入力されたメッセージを送信する */
void send_msg_from_keyboard(int sock, char *p_buf) {
    int strsize;
    char s_buf[MSGBUF_SIZE];
    strsize = strlen(p_buf);
    /* 改行を追加する */
    p_buf[strsize] = '\n';
    p_buf[strsize + 1] = '\0';
    /* 入力文字列先頭4文字がQUITだったら */
    if (strncmp(p_buf, "QUIT", 4) == 0) {
        /* QUITパケットを作成する */
        create_packet(s_buf, QUIT, "");
    } else {
        /* POST パケットを作成する */
        create_packet(s_buf, POST, p_buf);
    }
    strsize = strlen(s_buf);
    /* 送信 */
    Send(sock, s_buf, strsize, 0);
}
