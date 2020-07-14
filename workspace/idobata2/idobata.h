#ifndef IDOBATA_H
#define IDOBATA_H

#include <locale.h>
#include <ncurses.h>

/* Color pair */
#define COL_BLK_WHT 1
#define COL_GRN_WHT 2
#define COL_CYN_WHT 3

#define USERNAME_LEN 15 /* サーバ名格納用バッファサイズ */

/* ウィンドウを作成する */
void create_window(WINDOW **win_main, WINDOW **win_sub);
/* サーバーが既にあるか検索する */
int search_server(int port_num);
/* サーバーを起動する */
int idobata_server(int port_number);
/* クライアントを起動する */
void idobata_client(int port_number);

#endif
