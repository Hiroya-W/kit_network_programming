#ifndef IDOBATA_H
#define IDOBATA_H

#include <locale.h>
#include <ncurses.h>

/* Color pair */
#define COL_BLK_WHT 1
#define COL_GRN_WHT 2
#define COL_CYN_WHT 3

/* Packet Type */
#define HELLO 1
#define HERE 2
#define JOIN 3
#define POST 4
#define MESSAGE 5
#define QUIT 6

/* UDP通信でタイムアウトを使用する */
#define TIMEOUT_SEC 2

/* メッセージ全体のサイズは512バイト */
#define MSGBUF_SIZE 512
/* パケット内においてデータが入る部分は488バイト */
#define MSGDATA_SIZE 488
/* サーバ名格納用バッファサイズ */
#define USERNAME_LEN 15

/* ウィンドウを作成する */
void create_window(WINDOW **win_main, WINDOW **win_sub);
/* サーバーが既にあるか検索する */
int search_server(int port_num);
/* サーバーを起動する */
int idobata_server(int port_number);
/* クライアントを起動する */
void idobata_client(int port_number);

#endif
