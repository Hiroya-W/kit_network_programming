#include <curses.h>
#include <string.h>

#include "idobata.h"
#include "mynet.h"

static WINDOW *win_main, *win_sub;

static void init(int port_number, int *server_sock, int *client_sock);

void idobata_server(int port_number) {
    int server_sock;
    int client_sock;
    /* Windowを作成 */
    create_window(&win_main, &win_sub);
    wprintw(win_main, "サーバーモードで起動しました。\n");
    /* サーバー用の初期設定を行う*/
    init(port_number, &server_sock, &client_sock);
}

static void init(int port_number, int *server_sock, int *client_sock) {
    /* サーバーを起動する */
    *server_sock = init_tcpserver(port_number, 5);
    wprintw(win_main, "サーバーを起動しました。\n");
    /* 自分自身がサーバーなのでlocalhostを格納 */
    set_server_addr("localhost");
    /* 自分自身もサーバーに参加する */
    *client_sock = join_server(port_number);
    wprintw(win_main, "サーバーに参加しました。\n");
    /* 画面更新 */
    wrefresh(win_main);
    wrefresh(win_sub);
}

