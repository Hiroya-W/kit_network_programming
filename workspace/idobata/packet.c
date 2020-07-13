#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <locale.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "idobata.h"
#include "mynet.h"

#define MSGBUF_SIZE 512

#define HELLO 1
#define HERE 2
#define JOIN 3
#define POST 4
#define MESSAGE 5
#define QUIT 6

#define ERROR -1
#define SERVER_NOT_EXIST 0
#define SERVER_EXIST 1

#define TIMEOUT_SEC 2

#define SUBWIN_LINES 5 /* サブウィンドウの行数 */

static char Buffer[MSGBUF_SIZE];
char server_addr[20];
static int Max_sd = 0; /* ディスクリプタ最大値 */

int search_server(int port_num);
void create_packet(unsigned int type, char *message);
void send_packet(member_t user);
int idobata_server(int port_number);

void show_adrsinfo(struct sockaddr_in *adrs_in);
void join_message(char *username);
void send_message(char *message, char *username, int sock_from);

unsigned int analyze_header(char *header);

static void setMax_sd(int num);

static void init_color_pair();
static void create_window(WINDOW **win_main, WINDOW **win_sub);

/*
  パケットの種類=type のパケットを作成する
  パケットのデータは 内部的なバッファ(Buffer)に作成される
*/
void create_packet(unsigned int type, char *message) {
    switch (type) {
        case HELLO:
            snprintf(Buffer, MSGBUF_SIZE, "HELO");
            break;
        case HERE:
            snprintf(Buffer, MSGBUF_SIZE, "HERE");
            break;
        case JOIN:
            snprintf(Buffer, MSGBUF_SIZE, "JOIN %s", message);
            break;
        case POST:
            snprintf(Buffer, MSGBUF_SIZE, "POST %s", message);
            break;
        case MESSAGE:
            snprintf(Buffer, MSGBUF_SIZE, "MESG %s", message);
            break;
        case QUIT:
            snprintf(Buffer, MSGBUF_SIZE, "QUIT");
            break;
        default:
            /* Undefined packet type */
            break;
    }
}

int search_server(int port_num) {
    struct sockaddr_in broadcast_adrs;
    struct sockaddr_in from_adrs;
    ido_packet *packet;
    socklen_t from_len;

    int sock;
    int broadcast_sw = 1;
    fd_set mask, readfds;
    struct timeval timeout;

    char r_buf[MSGBUF_SIZE];
    int strsize;

    int is_server_found = 0;
    int cnt_retry = 0;

    /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
    set_sockaddr_in_broadcast(&broadcast_adrs, (in_port_t)port_num);

    /* ソケットをDGRAMモードで作成する */
    sock = init_udpclient();

    /* ソケットをブロードキャスト可能にする */
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1) {
        exit_errmesg("setsockopt()");
    }

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(sock, &mask);

    /* HELOパケットを作成 */
    create_packet(HELLO, "");
    strsize = strlen(Buffer);

    /* サーバから文字列を受信して表示 */
    while (cnt_retry < 3 && !is_server_found) {
        /* 文字列をサーバに送信する */
        Sendto(sock, Buffer, strsize, 0, (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs));
        /* 受信データの有無をチェック */
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        if (select(sock + 1, &readfds, NULL, NULL, &timeout) == 0) {
            printf("Time out.\n");
            cnt_retry++;
            continue;
        }

        /* 受信する */
        from_len = sizeof(from_adrs);
        strsize = Recvfrom(sock, r_buf, MSGBUF_SIZE - 1, 0, (struct sockaddr *)&from_adrs, &from_len);
        r_buf[strsize] = '\0';

        packet = (ido_packet *)r_buf; /* packetがバッファの先頭を指すようにする */
        if (analyze_header(packet->header) == HERE) {
            /* Serverからの返事があった時 */
            is_server_found = 1;
            strncpy(server_addr, inet_ntoa(from_adrs.sin_addr), 20);  // サーバーアドレスを保管する
            printf("Server already exists. [%s]\n", server_addr);
        } else {
            /* HELLOパケットに対する返事ではない */
            printf("This packet is not HERE packet.\n");
        }
    }

    close(sock); /* ソケットを閉じる */

    if (is_server_found) {
        return SERVER_EXIST;
    } else {
        printf("Server doesn't exist.\n");
        return SERVER_NOT_EXIST;
    }
}

int idobata_server(int port_number) {
    fd_set mask, readfds;
    struct sockaddr_in from_adrs;
    ido_packet *packet;
    int udp_sock;
    int tcp_sock;
    unsigned int from_len;

    char r_buf[MSGBUF_SIZE];
    int strsize;

    /* UDPサーバの初期化 */
    udp_sock = init_udpserver((in_port_t)port_number);
    setMax_sd(udp_sock);
    /* TCPサーバの初期化 */
    tcp_sock = init_tcpserver(port_number, 5);
    setMax_sd(tcp_sock);

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(udp_sock, &mask);
    FD_SET(tcp_sock, &mask);

    while (1) {
        readfds = mask;
        select(Max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(udp_sock, &readfds)) {
            // UDPでパケットを受信したらサーバーを探しているクライアントがいるので
            /* 文字列をクライアントから受信する */
            from_len = sizeof(from_adrs);
            strsize = Recvfrom(udp_sock, r_buf, MSGBUF_SIZE, 0, (struct sockaddr *)&from_adrs, &from_len);

            show_adrsinfo((struct sockaddr_in *)&from_adrs);

            packet = (ido_packet *)r_buf; /* packetがバッファの先頭を指すようにする */
            if (analyze_header(packet->header) == HELLO) {
                /* 文字列をクライアントに送信する */
                /* HELOパケットを作成 */
                create_packet(HERE, "");
                strsize = strlen(Buffer);

                /* HEREパケットをを送信者に送信する */
                Sendto(udp_sock, Buffer, strsize, 0, (struct sockaddr *)&from_adrs, sizeof(from_adrs));
            } else {
                /* HELLOパケットではないので、返事はしない */
                printf("This packet is not HELLO packet.\n");
            }
        } else if (FD_ISSET(tcp_sock, &readfds)) {
            // tcp_sockにデータがあったら
            // 新しいクライアントがコネクション張ろうとしているので
            /* クライアントの接続を受け付ける */
            int sock_accepted = Accept(tcp_sock, NULL, NULL);
            /* 名前は設定せずに登録 */
            add_node_to_list("", sock_accepted);

            /* クライアントとの通信を確認するため、ビットマスクをセット */
            FD_SET(sock_accepted, &mask);
            setMax_sd(sock_accepted);
        } else {
            // 特定のユーザからメッセージが届いている
            member_t current = get_head_from_list();
            // 全メンバーのsockを確認する
            while (1) {
                int client_sock = current->sock;

                if (FD_ISSET(client_sock, &readfds)) {
                    // このユーザから受信する
                    strsize = Recv(client_sock, r_buf, MSGBUF_SIZE - 1, 0);
                    r_buf[strsize] = '\0';
                    packet = (ido_packet *)r_buf;
                    // 名前が未設定だったら
                    if (strlen(current->username) == 0) {
                        // 名前が設定されるまでここにくる
                        // パケットtypeはJOINである必要がある
                        if (analyze_header(packet->header) == JOIN) {
                            // 名前を設定する
                            snprintf(current->username, USERNAME_LEN, "%s", packet->data);
                            join_message(current->username);
                        }
                        // JOIN以外は受け付けない
                    }
                    // 名前が設定されていたら
                    else {
                        switch (analyze_header(packet->header)) { /* ヘッダに応じて分岐 */
                            case HELLO:
                                /* HELOパケットを受けとった時の処理 */
                                // 何もしない
                                break;
                            case JOIN:
                                /* 以下、新規メンバー登録処理 */
                                // 何もしない
                                break;
                            case POST:
                                // 全員にメッセージを送信する
                                // send_message(int sock_from);
                                break;
                            case MESSAGE:
                                break;
                            case QUIT:
                                break;
                            default:
                                break;
                        }
                    }
                }

                if (current->next != NULL) {
                    // 次の場所がNULLではない、つまり次のノードがある。
                    // 現在の場所を次のノードに移動します。
                    current = current->next;
                } else {
                    // 次の場所がNULLなら、現在の場所はリストの末尾。
                    // もう次のデータはないので、breakでループ終了。
                    break;
                }
            }
        }
    }

    close(udp_sock);

    exit(EXIT_SUCCESS);
}

void join_message(char *username) {
    char username_message[MSGBUF_SIZE];
    // ユーザ名を付加したメッセージを生成
    snprintf(username_message, MSGBUF_SIZE, "%s が参加しました", username);
    printf("%s\n", username_message);
    send_message(username_message, "Server", -1);
}

void send_message(char *message, char *username, int sock_from) {
    member_t current = get_head_from_list();

    char username_message[MSGBUF_SIZE];
    // ユーザ名を付加したメッセージを生成
    snprintf(username_message, MSGBUF_SIZE, "[%s]%s", username, message);

    // メッセージにヘッダをつける
    create_packet(MESSAGE, username_message);

    while (1) {
        // メッセージの送信者以外に送信する
        if (current->sock != sock_from) {
            Send(current->sock, Buffer, MSGBUF_SIZE, 0);
        }

        if (current->next != NULL) {
            // 次の場所がNULLではない、つまり次のノードがある。
            // 現在の場所を次のノードに移動します。
            current = current->next;
        } else {
            // 次の場所がNULLなら、現在の場所はリストの末尾。
            // もう次のデータはないので、breakでループ終了。
            break;
        }
    }
}

void show_adrsinfo(struct sockaddr_in *adrs_in) {
    int port_number;
    char ip_adrs[20];

    strncpy(ip_adrs, inet_ntoa(adrs_in->sin_addr), 20);
    port_number = ntohs(adrs_in->sin_port);

    printf("%s[%d]\n", ip_adrs, port_number);
}

unsigned int analyze_header(char *header) {
    if (strncmp(header, "HELO", 4) == 0) return (HELLO);
    if (strncmp(header, "HERE", 4) == 0) return (HERE);
    if (strncmp(header, "JOIN", 4) == 0) return (JOIN);
    if (strncmp(header, "POST", 4) == 0) return (POST);
    if (strncmp(header, "MESG", 4) == 0) return (MESSAGE);
    if (strncmp(header, "QUIT", 4) == 0) return (QUIT);
    return 0;
}

/* Max_sdの値は常に最大値を取るようにする*/
static void setMax_sd(int num) {
    if (num > Max_sd) {
        Max_sd = num;
    }
}

void idobata_client(int port_number) {
    int sock;
    char s_buf[488], r_buf[488];
    fd_set mask, readfds;

    WINDOW *win_main, *win_sub;

    /* Windowを作る */
    create_window(&win_main, &win_sub);

    /* サーバに接続する */
    sock = init_tcpclient(server_addr, port_number);
    wprintw(win_main, "Mode: Client\n");
    wprintw(win_main, "Connected.\n");

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
            int strsize;
            int num_jp;
            char buf[512];
            /* サブウィンドウでキーボードから文字列を入力する */
            wgetnstr(win_sub, s_buf, 488 - 2);
            strsize = strlen(s_buf);
            /* 改行を追加する */
            s_buf[strsize] = '\n';
            s_buf[strsize + 1] = '\0';
            Send(sock, s_buf, strsize + 1, 0);

            /* 入力用のプロンプトを表示する */
            wprintw(win_sub, "> ");

            /* 自分にもメッセージを表示する */
            /* 日本語の文字数を数える */
            num_jp = cnt_jp(s_buf);
            snprintf(buf, MSGBUF_SIZE, "[You]: %s", s_buf);
            /* 右寄せで表示する */
            wattron(win_main, COLOR_PAIR(COL_GRN_WHT));
            wprintw(win_main, "%*s\n", COLS + num_jp, buf);
            wattroff(win_main, COLOR_PAIR(COL_GRN_WHT));
        }

        /* Chat-Serverからメッセージを受信した時 */
        if (FD_ISSET(sock, &readfds)) {
            /* サーバから文字列を受信する */
            int strsize = Recv(sock, r_buf, MSGBUF_SIZE - 1, 0);
            /* サーバから切断されたら */
            if (strsize == 0) {
                wprintw(win_main, "Chat-Server is down.\nDisconnected.\nPress Any key to exit.");
                wrefresh(win_main);
                close(sock);
                /* 何かのキー入力を待つ */
                wgetch(win_sub);
                return;
            }
            r_buf[strsize] = '\0';
            /* 通常はwin_mainに出力 */
            wattron(win_main, COLOR_PAIR(COL_CYN_WHT));
            wprintw(win_main, r_buf);
            wattroff(win_main, COLOR_PAIR(COL_CYN_WHT));
        }
    }
}

/* WINDOWを作成 */
static void create_window(WINDOW **win_main, WINDOW **win_sub) {
    /* 日本語を表示できるように */
    /* initする前に書く */
    setlocale(LC_ALL, "");

    /* 画面の初期化 */
    if (initscr() == NULL) {
        exit_errmesg("initscr()");
    }

    /* カラーが使えるか確認する */
    if (has_colors() == FALSE) {
        endwin();
        exit_errmesg("Terminal does not support color.");
    }

    *win_main = newwin(LINES - SUBWIN_LINES, COLS, 0, 0);
    if (*win_main == NULL) {
        exit_errmesg("newwin()");
    }

    *win_sub = newwin(SUBWIN_LINES, COLS, LINES - SUBWIN_LINES, 0);
    if (*win_sub == NULL) {
        exit_errmesg("newwin()");
    }

    /* 文字色を付けられるように */
    start_color();
    /* 端末デフォルトの文字色と背景色を-1でアクセスできるようにする */
    use_default_colors();
    init_color_pair();

    /* 背景色を設定 */
    wbkgd(*win_main, COLOR_PAIR(COL_BLK_WHT));
    wbkgd(*win_sub, COLOR_PAIR(COL_BLK_WHT));
    /* 画面を更新 */
    wrefresh(*win_main);
    wrefresh(*win_sub);
    /* スクロールを許可する */
    scrollok(*win_main, TRUE);
    scrollok(*win_sub, TRUE);

    return;
}

/* Color pairの定義 */
static void init_color_pair() {
    init_pair(COL_BLK_WHT, COLOR_BLACK, COLOR_WHITE);
    init_pair(COL_GRN_WHT, COLOR_GREEN, COLOR_WHITE);
    init_pair(COL_CYN_WHT, COLOR_CYAN, COLOR_WHITE);
}

/* 日本語の出現回数をカウントする関数 */
int cnt_jp(char *str) {
    /* UTF8で日本語は3バイト */
    /* 2byte目移行なら (*str & 0xC0) == 0x80 となる */
    /* 結果的に日本語のときにだけcountが2増える */
    int count = 0;
    while (*str != '\0') {
        if ((*str & 0xC0) == 0x80) {
            count++;
        }
        str++;
    }
    return count / 2;
}
