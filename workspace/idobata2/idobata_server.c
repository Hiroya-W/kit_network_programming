#include <curses.h>
#include <string.h>
#include <sys/select.h>

#include "idobata.h"
#include "mynet.h"

/* Window */
static WINDOW *win_main, *win_sub;
/* selectで監視する最大値 */
static int Max_sd = 0;

static void init(int port_number, int *server_udp_sock, int *server_tcp_sock, int *client_tcp_sock);
static void recv_udp_packet(int udp_sock);
static void setMax_sd(int num);

void idobata_server(int port_number) {
    fd_set mask, readfds;
    int server_udp_sock;
    int server_tcp_sock;
    int client_tcp_sock;
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
                wprintw(win_main, "井戸端サーバーが終了しました。\nキー入力でクライアントを終了します。\n");
                wrefresh(win_main);
                close(client_tcp_sock);
                /* 何かのキー入力を待つ */
                wgetch(win_sub);
                return;
            }
            r_buf[strsize] = '\0';
            show_others_msg(win_main, r_buf);
        } else {
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

    show_adrsinfo((struct sockaddr_in *)&from_adrs);

    packet = (ido_packet_t *)r_buf; /* packetがバッファの先頭を指すようにする */
    /* HELLOパケット以外は受け取らない */
    if (analyze_header(packet->header) != HELLO) {
        return;
    }
    /* 文字列をクライアントに送信する */
    /* HELOパケットを作成 */
    create_packet(s_buf, HELLO, "");
    strsize = strlen(s_buf);

    /* HEREパケットをを送信者に送信する */
    Sendto(udp_sock, s_buf, strsize, 0, (struct sockaddr *)&from_adrs, sizeof(from_adrs));
}

/* Max_sdの値は常に最大値を取るようにする*/
static void setMax_sd(int num) {
    if (num > Max_sd) {
        Max_sd = num;
    }
}
