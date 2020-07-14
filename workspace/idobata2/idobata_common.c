/*
 * idobata_common.c
 */

#include "idobata.h"
#include "mynet.h"

/*
  パケットの種類=type のパケットを作成する
  パケットのデータは 内部的なバッファ(Buffer)に作成される
*/
void create_packet(char *Buffer, unsigned int type, char *message) {
    switch (type) {
        case HELLO:
            snprintf(Buffer, MSGBUF_SIZE, "HELO");
            break;
        case HERE:
            snprintf(Buffer, MSGBUF_SIZE, "HERE");
            break;
        case JOIN:
            snprintf(Buffer, MSGBUF_SIZE, "JOIN %s", message);
            break;
        case POST:
            snprintf(Buffer, MSGBUF_SIZE, "POST %s", message);
            break;
        case MESSAGE:
            snprintf(Buffer, MSGBUF_SIZE, "MESG %s", message);
            break;
        case QUIT:
            snprintf(Buffer, MSGBUF_SIZE, "QUIT");
            break;
        default:
            /* Undefined packet type */
            break;
    }
}

/* パケットのタイプを解析する */
unsigned int analyze_header(char *header) {
    if (strncmp(header, "HELO", 4) == 0) return (HELLO);
    if (strncmp(header, "HERE", 4) == 0) return (HERE);
    if (strncmp(header, "JOIN", 4) == 0) return (JOIN);
    if (strncmp(header, "POST", 4) == 0) return (POST);
    if (strncmp(header, "MESG", 4) == 0) return (MESSAGE);
    if (strncmp(header, "QUIT", 4) == 0) return (QUIT);
    return 0;
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
