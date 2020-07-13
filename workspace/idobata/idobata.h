#ifndef IDOBATA_H
#define IDOBATA_H

/* Color pair */
#define COL_BLK_WHT 1
#define COL_GRN_WHT 2
#define COL_CYN_WHT 3

#define USERNAME_LEN 15 /* サーバ名格納用バッファサイズ */

typedef struct _imember {
    char username[USERNAME_LEN]; /* ユーザ名 */
    int sock;                    /* ソケット番号 */
    struct _imember *next;       /* 次のユーザ */
} * member_t;

typedef struct _idobata {
    char header[4]; /* パケットのヘッダ部(4バイト) */
    char sep;       /* セパレータ(空白、またはゼロ) */
    char data[488]; /* データ部分(メッセージ本体) */
} ido_packet;

/* メンバーの線形リストの先頭を取得する */
member_t get_head_from_list();
member_t create_member();
void add_node_to_list(char *username, int sock);
/* メンバーを線形リストに追加する */
void add_node_to_list(char *username, int sock);
/* サーバーが既にあるか検索する */
int search_server(int port_num);
/* サーバーを起動する */
int idobata_server(int port_number);
/* クライアントを起動する */
void idobata_client(int port_number);

/* 日本語の出現回数をカウントする関数 */
int cnt_jp(char *str);

#endif
