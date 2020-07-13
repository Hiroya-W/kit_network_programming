#ifndef IDOBATA_H
#define IDOBATA_H

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

/* サーバーが既にあるか検索する */
int search_server(int port_num);
/* サーバーを起動する */
int idobata_server(int port_number);

#endif
