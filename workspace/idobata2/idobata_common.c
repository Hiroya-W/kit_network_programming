/*
 * idobata_common.c
 */

#include <stdio.h>

#include "idobata.h"
#include "mynet.h"

char server_addr[20];
char user_name[USERNAME_LEN];

/* サーバーアドレスを格納する */
void set_server_addr(char *addr) {
    snprintf(server_addr, 20, "%s", addr);
}

/* 格納されているサーバーアドレスを取得する */
void get_server_addr(char *out) {
    snprintf(out, 20, "%s", server_addr);
}

/* ユーザー名を格納する */
void set_user_name(char *name) {
    snprintf(user_name, USERNAME_LEN, "%s", name);
}

/* 格納されているユーザ名を取得する */
void get_user_name(char *out) {
    snprintf(out, USERNAME_LEN, "%s", user_name);
}

/* 格納されているアドレス情報を表示する */
void show_adrsinfo(struct sockaddr_in *adrs_in) {
    int port_number;
    char ip_adrs[20];

    strncpy(ip_adrs, inet_ntoa(adrs_in->sin_addr), 20);
    port_number = ntohs(adrs_in->sin_port);

    printf("%s[%d]\n", ip_adrs, port_number);
}

/* 日本語の出現回数をカウントする関数 */
int cnt_jp(char *str) {
    /* UTF8で日本語は3バイト */
    /* 2byte目移行なら (*str & 0xC0) == 0x80 となる */
    /* 結果的に日本語のときにだけcountが2増える */
    int count = 0;
    while (*str != '\0') {
        if ((*str & 0xC0) == 0x80) {
            count++;
        }
        str++;
    }
    return count / 2;
}
