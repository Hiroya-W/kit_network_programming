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
    fd_set mask, readfds;
    int server_udp_sock;
    int server_tcp_sock;
    int client_tcp_sock;
    /* Windowを作成 */
    create_window(&win_main, &win_sub);
    wprintw(win_main, "サーバーモードで起動しました。\n");
    /* サーバー用の初期設定を行う*/
    init(port_number, &server_udp_sock, &server_tcp_sock, &client_tcp_sock);

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(0, &mask);
    // FD_SET(server_udp_sock, &mask);
    // FD_SET(server_tcp_sock, &mask);
    // FD_SET(client_tcp_sock, &mask);

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
    }
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
}

/* Max_sdの値は常に最大値を取るようにする*/
static void setMax_sd(int num) {
    if (num > Max_sd) {
        Max_sd = num;
    }
}
