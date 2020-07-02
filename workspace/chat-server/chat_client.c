/*
  chat_client.c
*/
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "chat.h"
#include "mynet.h"

#define S_BUFSIZE 500 /* 送信用バッファサイズ */
#define R_BUFSIZE 500 /* 受信用バッファサイズ */

#define SUBWIN_LINES 5 /* サブウィンドウの行数 */

static void init_color_pair();
static void create_window(WINDOW **win_main, WINDOW **win_sub);

void chat_client(char *servername, int port_number) {
    int sock;
    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int name_is_set = 0;
    fd_set mask, readfds;

    WINDOW *win_main, *win_sub;

    /* Windowを作る */
    create_window(&win_main, &win_sub);

    /* サーバに接続する */
    sock = init_tcpclient(servername, port_number);
    wprintw(win_main, "Mode: Client\n");
    wprintw(win_main, "Server name: %s\n", servername);
    wprintw(win_main, "Server Port: %d\n", port_number);
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
            char buf[S_BUFSIZE + 100];
            /* サブウィンドウでキーボードから文字列を入力する */
            wgetnstr(win_sub, s_buf, S_BUFSIZE - 2);
            strsize = strlen(s_buf);
            /* 改行を追加する */
            s_buf[strsize] = '\n';
            s_buf[strsize + 1] = '\0';
            Send(sock, s_buf, strsize + 1, 0);

            /* 入力用のプロンプトを表示する */
            wprintw(win_sub, "> ");
            /* 初回の名前入力時は入力した文字列を表示しない */
            if (!name_is_set) {
                name_is_set = 1;
                continue;
            }

            /* 自分にもメッセージを表示する */
            /* 日本語の文字数を数える */
            num_jp = cnt_jp(s_buf);
            snprintf(buf, S_BUFSIZE + 100, "[You]: %s", s_buf);
            /* 右寄せで表示する */
            wattron(win_main, COLOR_PAIR(COL_GRN_WHT));
            wprintw(win_main, "%*s\n", COLS + num_jp, buf);
            wattroff(win_main, COLOR_PAIR(COL_GRN_WHT));
        }

        /* Chat-Serverからメッセージを受信した時 */
        if (FD_ISSET(sock, &readfds)) {
            /* サーバから文字列を受信する */
            int strsize = Recv(sock, r_buf, R_BUFSIZE - 1, 0);
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
            /* 初回の名前入力時はプロンプトをwin_subに表示する */
            if (!name_is_set) {
                wprintw(win_sub, r_buf);
                continue;
            }
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
