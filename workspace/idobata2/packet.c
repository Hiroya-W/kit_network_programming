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

