#include <curses.h>
#include <stdio.h>
#include <string.h>

#include "idobata.h"
#include "mynet.h"

static WINDOW *win_main, *win_sub;

void idobata_client(int port_number) {
    int sock;
    fd_set mask, readfds;

    /* Windowを作成 */
    create_window(&win_main, &win_sub);
    wprintw(win_main, "クライアントモードで起動しました。\n");
    /* クライアント用の初期設定を行う*/
    sock = join_server(port_number);
    /* 画面更新 */
    wprintw(win_main, "サーバーに参加しました。\n");

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(0, &mask);
    FD_SET(sock, &mask);

    /* メインループ */
    while (1) {
        wrefresh(win_main);
        wrefresh(win_sub);
        /* 受信データの有無をチェック */
        readfds = mask;
        select(sock + 1, &readfds, NULL, NULL, NULL);

        /* キーボードからの入力があった時 */
        if (FD_ISSET(0, &readfds)) {
            send_msg_from_keyboard(sock);
        }
    }
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

/* キーボードから入力を受け取り、送信する */
void send_msg_from_keyboard(int sock) {
    int strsize;
    int num_jp;
    char p_buf[MSGBUF_SIZE];
    char s_buf[MSGBUF_SIZE];
    /* サブウィンドウでキーボードから文字列を入力する */
    /* 入力出来る文字は488バイトで、うち2バイトは改行とヌル文字にする */
    wgetnstr(win_sub, p_buf, MSGDATA_SIZE - 2);
    strsize = strlen(p_buf);
    /* 改行を追加する */
    p_buf[strsize] = '\n';
    p_buf[strsize + 1] = '\0';
    snprintf(s_buf, MSGDATA_SIZE, "%s", p_buf);

    /* MESSAGE パケットを作成する */
    create_packet(s_buf, MESSAGE, s_buf);
    strsize = strlen(s_buf);
    /* 送信 */
    Send(sock, s_buf, strsize, 0);

    /* 入力用のプロンプトを表示する */
    wprintw(win_sub, "> ");

    /* 自分にもメッセージを表示する */
    /* 日本語の文字数を数える */
    num_jp = cnt_jp(p_buf);
    snprintf(p_buf, MSGBUF_SIZE, "[You]: %s", p_buf);
    /* 右寄せで表示する */
    wattron(win_main, COLOR_PAIR(COL_GRN_WHT));
    wprintw(win_main, "%*s\n", COLS + num_jp, p_buf);
    wattroff(win_main, COLOR_PAIR(COL_GRN_WHT));
}
