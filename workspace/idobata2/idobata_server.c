#include <curses.h>
#include <string.h>

#include "idobata.h"
#include "mynet.h"

/* Window */
static WINDOW *win_main, *win_sub;
/* selectで監視する最大値 */
static int Max_sd = 0;

static void init(int port_number, int *server_udp_sock, int *server_tcp_sock, int *client_tcp_sock);
static void setMax_sd(int num);

void idobata_server(int port_number) {
    int server_udp_sock;
    int server_tcp_sock;
    int client_tcp_sock;
    /* Windowを作成 */
    create_window(&win_main, &win_sub);
    wprintw(win_main, "サーバーモードで起動しました。\n");
    /* サーバー用の初期設定を行う*/
    init(port_number, &server_udp_sock, &server_tcp_sock, &client_tcp_sock);
}

static void init(int port_number, int *server_udp_sock, int *server_tcp_sock, int *client_tcp_sock) {
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
    /* 画面更新 */
    wrefresh(win_main);
    wrefresh(win_sub);
}

/* Max_sdの値は常に最大値を取るようにする*/
static void setMax_sd(int num) {
    if (num > Max_sd) {
        Max_sd = num;
    }
}
