#ifndef IDOBATA_H
#define IDOBATA_H

#include <arpa/inet.h>
#include <locale.h>
#include <ncurses.h>

/* サーバー検索時の返り値 */
#define SERVER_NOT_EXIST 0
#define SERVER_EXIST 1
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

typedef struct _imember {
    char username[USERNAME_LEN]; /* ユーザ名 */
    int sock;                    /* ソケット番号 */
    struct _imember *next;       /* 次のユーザ */
} * member_t;

/* パケットの構造 */
typedef struct _idobata {
    char header[4]; /* パケットのヘッダ部(4バイト) */
    char sep;       /* セパレータ(空白、またはゼロ) */
    char data[488]; /* データ部分(メッセージ本体) */
} ido_packet_t;

// =============================================
//  window.c
// =============================================

/* ウィンドウを作成する */
void create_window(WINDOW **win_main, WINDOW **win_sub);

// =============================================
//  idobata_common.c
// =============================================
int search_server(int port_number);
/* サーバーアドレスを格納する */
void set_server_addr(char *addr);

/* 格納されているサーバーアドレスを取得する */
void get_server_addr(char *out);
/* ユーザー名を格納する */
void set_user_name(char *name);
/* 格納されているユーザ名を取得する */
void get_user_name(char *out);
/*
  パケットの種類=type のパケットを作成する
  パケットのデータは 内部的なバッファ(Buffer)に作成される
*/
void create_packet(char *Buffer, unsigned int type, char *message);
/* パケットのタイプを解析する */
unsigned int analyze_header(char *header);
/* 格納されているアドレス情報を表示する */
void show_adrsinfo(struct sockaddr_in *adrs_in);
/* 自分が送信したメッセージを表示する */
void show_your_msg(WINDOW *win, char *buf);
/* 他の人の送信したメッセージを表示する */
void show_others_msg(WINDOW *win, char *buf);
/* 日本語の出現回数をカウントする関数 */
int cnt_jp(char *str);

// =============================================
//  idobata_server.c
// =============================================
/* サーバーを起動する */
void idobata_server(int port_number);

// =============================================
//  idobata_client.c
// =============================================
/* クライアントを起動する */
void idobata_client(int port_number);
/* サーバーに参加する */
int join_server(int port_number);
/* キーボード入力されたメッセージを送信する */
void send_msg_from_keyboard(int sock, char *p_buf);

// =============================================
//  linear_lists.c
// =============================================
/* 線形リストの先頭を取得する */
member_t get_head_from_list();
/* 線形リストにユーザを追加する */
void add_user_to_list(char *username, int sock);
/* 線形リストからユーザを削除する */
void delete_user_from_list(int sock);
#endif
