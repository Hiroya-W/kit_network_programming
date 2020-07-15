/*
 * idobata_common.c
 */

#include <stdio.h>
#include <string.h>

#include "idobata.h"
#include "mynet.h"

char server_addr[20];
char user_name[USERNAME_LEN];

int search_server(int port_number) {
    struct sockaddr_in broadcast_adrs;
    struct sockaddr_in from_adrs;

    ido_packet_t *packet;
    socklen_t from_len;

    int sock;
    int broadcast_sw = 1;

    fd_set mask, readfds;
    struct timeval timeout;

    char s_buf[MSGBUF_SIZE];
    char r_buf[MSGBUF_SIZE];
    int strsize;

    int is_server_found = 0;
    int cnt_retry = 0;

    /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
    set_sockaddr_in_broadcast(&broadcast_adrs, (in_port_t)port_number);
    /* ソケットをDGRAMモードで作成する */
    sock = init_udpclient();
    /* ソケットをブロードキャスト可能にする */
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1) {
        exit_errmesg("setsockopt()");
    }

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(sock, &mask);

    /* HELOパケットを作成する */
    create_packet(s_buf, HELLO, "");
    strsize = strlen(s_buf);

    printf("井戸端会議サーバーを検索します。\n");
    printf("タイムアウト上限回数 3回\n");
    while (cnt_retry < 3 && !is_server_found) {
        /* 文字列をサーバに送信する */
        Sendto(sock, s_buf, strsize, 0, (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs));
        /* 受信データの有無をチェック */
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        if (select(sock + 1, &readfds, NULL, NULL, &timeout) == 0) {
            printf("[%d] Time out.\n", cnt_retry + 1);
            cnt_retry++;
            continue;
        }

        /* 受信する */
        from_len = sizeof(from_adrs);
        strsize = Recvfrom(sock, r_buf, MSGBUF_SIZE - 1, 0, (struct sockaddr *)&from_adrs, &from_len);
        r_buf[strsize] = '\0';

        /* packetがバッファの先頭を指すようにする */
        packet = (ido_packet_t *)r_buf;
        if (analyze_header(packet->header) == HERE) {
            /* Serverからの返事があった時 */
            is_server_found = 1;
            // サーバーアドレスを保管する
            set_server_addr(inet_ntoa(from_adrs.sin_addr));
            printf("井戸端会議サーバーが見つかりました。[%s]\n", server_addr);
        } else {
            /* HELLOパケットに対する返事ではない */
            printf("HEREではないパケットを受信しました。\n");
        }
    }

    close(sock); /* ソケットを閉じる */

    if (is_server_found) {
        return SERVER_EXIST;
    } else {
        printf("井戸端会議サーバーは見つかりませんでした。\n");
        return SERVER_NOT_EXIST;
    }
}

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

/* 自分が送信したメッセージを表示する */
void show_your_msg(WINDOW *win, char *buf) {
    char p_buf[MSGBUF_SIZE + 10];
    /* 日本語の文字数を数える */
    int num_jp = cnt_jp(buf);
    snprintf(p_buf, MSGBUF_SIZE + 10, "[You]: %s", buf);
    /* 右寄せで表示する */
    wattron(win, COLOR_PAIR(COL_GRN_WHT));
    wprintw(win, "%*s\n", COLS + num_jp, buf);
    wattroff(win, COLOR_PAIR(COL_GRN_WHT));
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
