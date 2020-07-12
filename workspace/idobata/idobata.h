#ifndef IDOBATA_H
#define IDOBATA_H

#define USERNAME_LEN 15 /* サーバ名格納用バッファサイズ */

typedef struct _imember {
    char username[USERNAME_LEN]; /* ユーザ名 */
    int sock;                    /* ソケット番号 */
    struct _imember *next;       /* 次のユーザ */
} * member_t;

#endif
