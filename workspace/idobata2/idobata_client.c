#include <curses.h>
#include <string.h>

#include "idobata.h"
#include "mynet.h"

static WINDOW *win_main, *win_sub;

int join_server(int port_number);

void idobata_client(int port_number) {
    int sock;
    /* Windowを作成 */
    create_window(&win_main, &win_sub);
    wprintw(win_main, "クライアントモードで起動しました。\n");
    /* クライアント用の初期設定を行う*/
    sock = join_server(port_number);
    /* 画面更新 */
    wprintw(win_main, "サーバーに参加しました。\n");
    wrefresh(win_main);
    wrefresh(win_sub);
}

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

