/*
 * window.c
 */
#include <stdio.h>
#include <stdlib.h>

#include "idobata.h"
#include "mynet.h"

#define SUBWIN_LINES 5 /* サブウィンドウの行数 */

static void init_color_pair();

/* WINDOWを作成 */
void create_window(WINDOW **win_main, WINDOW **win_sub) {
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
