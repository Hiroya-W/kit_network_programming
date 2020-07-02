/*
 * chat.h
*/

#ifndef CHAT_H_
#define CHAT_H_

#include "mynet.h"

/* Color pair */
#define COL_BLK_WHT 1
#define COL_GRN_WHT 2
#define COL_CYN_WHT 3

/* サーバメインルーチン */
void chat_server(int port_number, int n_client);

/* クライアントメインルーチン */
void chat_client(char *servername, int port_number);

/* Accept関数(エラー処理つき) */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/* 送信関数(エラー処理つき) */
int Send(int s, void *buf, size_t len, int flags);

/* 受信関数(エラー処理つき) */
int Recv(int s, void *buf, size_t len, int flags);

/* 日本語の出現回数をカウントする関数 */
int cnt_jp(char *str);
#endif /* CHAT_H_ */
