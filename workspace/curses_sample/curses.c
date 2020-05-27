#include <curses.h>

#include "mynet.h"

#define PORT 50000    /* ポート番号 */
#define S_BUFSIZE 128 /* 送信用バッファサイズ */
#define R_BUFSIZE 128 /* 受信用バッファサイズ */

#define SUBWIN_LINES 5 /* サブウィンドウの行数 */

int main() {
    int sock;
    char servername[] = "localhost";
    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int strsize;
    WINDOW *win_main, *win_sub;

    /* ソケットの初期化 */
    sock = init_tcpclient(servername, PORT);

    /* 画面の初期化 */
    initscr();

    win_main = newwin(LINES - SUBWIN_LINES, COLS, 0, 0); /* Windowを作る */
    win_sub = newwin(SUBWIN_LINES, COLS, LINES - SUBWIN_LINES, 0);

    scrollok(win_main, TRUE); /* スクロールを許可する */
    scrollok(win_sub, TRUE);

    wmove(win_main, 0, 20);                         /* カーソルを動かす */
    wprintw(win_main, "TCP Echo Client Program\n"); /* 文字列を出力 */

    wrefresh(win_main); /* 画面を更新 */
    wrefresh(win_sub);

    /* サブウィンドウでキーボードから文字列を入力する */
    wgetnstr(win_sub, s_buf, S_BUFSIZE);

    strsize = strlen(s_buf);
    s_buf[strsize] = '\n'; /* 文字列の最後に改行コードを入れておく */

    /* 文字列をサーバに送信する */
    if (send(sock, s_buf, strsize + 1, 0) == -1) {
        exit_errmesg("send()");
    }

    /* サーバから文字列を受信する */
    do {
        if ((strsize = recv(sock, r_buf, R_BUFSIZE - 1, 0)) == -1) {
            exit_errmesg("recv()");
        }
        r_buf[strsize] = '\0';
        wprintw(win_main, "%s", r_buf); /* メインウィンドウに出力 */
    } while (r_buf[strsize - 1] != '\n');

    wrefresh(win_main);

    /* Yキーが押されたら終了する */
    wprintw(win_sub, "Hit Y key to exit.");
    wrefresh(win_sub);
    while (wgetch(win_sub) != 'Y')
        ;

    close(sock); /* ソケットを閉じる */
    endwin();    /* 画面の設定を元に戻す */

    exit(0);
}
