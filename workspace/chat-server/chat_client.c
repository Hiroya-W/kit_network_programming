/*
  chat_client.c
*/
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "chat.h"
#include "mynet.h"

#define S_BUFSIZE 100 /* 送信用バッファサイズ */
#define R_BUFSIZE 100 /* 受信用バッファサイズ */

#define SUBWIN_LINES 5 /* サブウィンドウの行数 */

WINDOW *Newwin(int lines, int cols, int posy, int posx);
void Wprintw(WINDOW *win, char *str);

void chat_client(char *servername, int port_number) {
    int sock;
    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int name_is_set = 0;
    fd_set mask, readfds;

    WINDOW *win_main, *win_sub;

    /* 画面の初期化 */
    initscr();

    /* Windowを作る */
    win_main = Newwin(LINES - SUBWIN_LINES, COLS, 0, 0);
    win_sub = Newwin(SUBWIN_LINES, COLS, LINES - SUBWIN_LINES, 0);

    /* サーバに接続する */
    sock = init_tcpclient(servername, port_number);
    Wprintw(win_main, "Connected.\n");

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(0, &mask);
    FD_SET(sock, &mask);

    for (;;) {
        /* 受信データの有無をチェック */
        readfds = mask;
        select(sock + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(0, &readfds)) {
            /* サブウィンドウでキーボードから文字列を入力する */
            wgetnstr(win_sub, s_buf, S_BUFSIZE);
            int strsize = strlen(s_buf);
            /* 改行を追加する */
            s_buf[strsize] = '\n';
            Send(sock, s_buf, strsize + 1, 0);

            /* 入力用のプロンプトを表示する */
            Wprintw(win_sub, "> ");
        }

        if (FD_ISSET(sock, &readfds)) {
            /* サーバから文字列を受信する */
            int strsize = Recv(sock, r_buf, R_BUFSIZE - 1, 0);
            /* サーバから切断されたら */
            if (strsize == 0) {
                Wprintw(win_main, "Chat-Server is down.\n");
                Wprintw(win_main, "Disconnected.\n");
                close(sock);
                return;
            }
            r_buf[strsize] = '\0';
            /* 初回の名前入力時はプロンプトをwin_subに表示する */
            if (!name_is_set) {
                name_is_set = 1;
                Wprintw(win_sub, r_buf);
                continue;
            }
            /* 通常はwin_mainに出力 */
            Wprintw(win_main, r_buf);
        }
    }
}

WINDOW *Newwin(int lines, int cols, int posy, int posx) {
    /* Windowを作る */
    WINDOW *win = newwin(lines, cols, posy, posx);
    /* 画面を更新 */
    wrefresh(win);
    /* スクロールを許可する */
    scrollok(win, TRUE);
    return win;
}

void Wprintw(WINDOW *win, char *str) {
    wprintw(win, str);
    /* 画面を更新 */
    wrefresh(win);
}
