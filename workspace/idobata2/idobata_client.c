#include <curses.h>
#include <string.h>

#include "idobata.h"
#include "mynet.h"

WINDOW *win_main, *win_sub;

static int init(int port_number);
static void show_client_mode_message();

void idobata_client(int port_number) {
    int sock;
    /* Windowを作成 */
    create_window(&win_main, &win_sub);
    /* クライアント用の初期設定を行う*/
    sock = init(port_number);
}

static int init(int port_number) {
    char server_addr[20];
    char user_name[USERNAME_LEN];
    char s_buf[MSGBUF_SIZE];
    int sock;
    int strsize;
    /* クライアントモードで起動した時に最初に表示するメッセージ */
    show_client_mode_message();
    /* 格納されているサーバーアドレスを取得する */
    get_server_addr(server_addr);
    /* 格納されているユーザ名を取得する */
    get_user_name(user_name);
    /* サーバーに接続する */
    sock = init_tcpclient(server_addr, port_number);
    wprintw(win_main, "サーバーに接続しました。");
    /* JOINパケットを送信する */
    create_packet(s_buf, JOIN, user_name);
    strsize = strlen(s_buf);
    Send(sock, s_buf, strsize, 0);
    return sock;
}

/* クライアントモードで起動した時に最初に表示するメッセージ */
static void show_client_mode_message() {
    wprintw(win_main, "井戸端会議サーバーが見つかりました。\n");
    wprintw(win_main, "クライアントモードで起動します。\n");
}
