#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "idobata.h"
#include "mynet.h"

#define MSGBUF_SIZE 512

#define HELLO 1
#define HERE 2
#define JOIN 3
#define POST 4
#define MESSAGE 5
#define QUIT 6

#define ERROR -1
#define SERVER_NOT_EXIST 0
#define SERVER_EXIST 1

#define TIMEOUT_SEC 2

static char Buffer[MSGBUF_SIZE];
char server_addr[20];
static int Max_sd = 0; /* ディスクリプタ最大値 */

int search_server(int port_num);
void create_packet(unsigned int type, char *message);
void send_packet(member_t user);
int idobata_server(int port_number);

void show_adrsinfo(struct sockaddr_in *adrs_in);

unsigned int analyze_header(char *header);

static void setMax_sd(int num);

/*
  パケットの種類=type のパケットを作成する
  パケットのデータは 内部的なバッファ(Buffer)に作成される
*/
void create_packet(unsigned int type, char *message) {
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

int search_server(int port_num) {
    struct sockaddr_in broadcast_adrs;
    struct sockaddr_in from_adrs;
    ido_packet *packet;
    socklen_t from_len;

    int sock;
    int broadcast_sw = 1;
    fd_set mask, readfds;
    struct timeval timeout;

    char r_buf[MSGBUF_SIZE];
    int strsize;

    int is_server_found = 0;
    int cnt_retry = 0;

    /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
    set_sockaddr_in_broadcast(&broadcast_adrs, (in_port_t)port_num);

    /* ソケットをDGRAMモードで作成する */
    sock = init_udpclient();

    /* ソケットをブロードキャスト可能にする */
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1) {
        exit_errmesg("setsockopt()");
    }

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(sock, &mask);

    /* HELOパケットを作成 */
    create_packet(HELLO, "");
    strsize = strlen(Buffer);

    /* サーバから文字列を受信して表示 */
    while (cnt_retry < 3 && !is_server_found) {
        /* 文字列をサーバに送信する */
        Sendto(sock, Buffer, strsize, 0, (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs));
        /* 受信データの有無をチェック */
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        if (select(sock + 1, &readfds, NULL, NULL, &timeout) == 0) {
            printf("Time out.\n");
            cnt_retry++;
            continue;
        }

        /* 受信する */
        from_len = sizeof(from_adrs);
        strsize = Recvfrom(sock, r_buf, MSGBUF_SIZE - 1, 0, (struct sockaddr *)&from_adrs, &from_len);
        r_buf[strsize] = '\0';

        packet = (ido_packet *)r_buf; /* packetがバッファの先頭を指すようにする */
        if (analyze_header(packet->header) == HERE) {
            /* Serverからの返事があった時 */
            is_server_found = 1;
            strncpy(server_addr, inet_ntoa(from_adrs.sin_addr), 20);  // サーバーアドレスを保管する
            printf("Server already exists. [%s]\n", server_addr);
        } else {
            /* HELLOパケットに対する返事ではない */
            printf("This packet is not HERE packet.\n");
        }
    }

    close(sock); /* ソケットを閉じる */

    if (is_server_found) {
        return SERVER_EXIST;
    } else {
        printf("Server doesn't exist.\n");
        return SERVER_NOT_EXIST;
    }
}

int idobata_server(int port_number) {
    fd_set mask, readfds;
    struct sockaddr_in from_adrs;
    ido_packet *packet;
    int udp_sock;
    int tcp_sock;
    unsigned int from_len;

    char r_buf[MSGBUF_SIZE];
    int strsize;

    /* UDPサーバの初期化 */
    udp_sock = init_udpserver((in_port_t)port_number);
    setMax_sd(udp_sock);
    /* TCPサーバの初期化 */
    tcp_sock = init_tcpserver(port_number, 5);
    setMax_sd(tcp_sock);

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(udp_sock, &mask);
    FD_SET(tcp_sock, &mask);

    while (1) {
        readfds = mask;
        select(Max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(udp_sock, &readfds)) {
            // UDPでパケットを受信したらサーバーを探しているクライアントがいるので
            /* 文字列をクライアントから受信する */
            from_len = sizeof(from_adrs);
            strsize = Recvfrom(udp_sock, r_buf, MSGBUF_SIZE, 0, (struct sockaddr *)&from_adrs, &from_len);

            show_adrsinfo((struct sockaddr_in *)&from_adrs);

            packet = (ido_packet *)r_buf; /* packetがバッファの先頭を指すようにする */
            if (analyze_header(packet->header) == HELLO) {
                /* 文字列をクライアントに送信する */
                /* HELOパケットを作成 */
                create_packet(HERE, "");
                strsize = strlen(Buffer);

                /* HEREパケットをを送信者に送信する */
                Sendto(udp_sock, Buffer, strsize, 0, (struct sockaddr *)&from_adrs, sizeof(from_adrs));
            } else {
                /* HELLOパケットではないので、返事はしない */
                printf("This packet is not HELLO packet.\n");
            }
        } else if (FD_ISSET(tcp_sock, &readfds)) {
            // tcp_sockにデータがあったら
            // 新しいクライアントがコネクション張ろうとしているので
            /* クライアントの接続を受け付ける */
            int sock_accepted = Accept(tcp_sock, NULL, NULL);
            /* 名前は設定せずに登録 */
            add_node_to_list("", sock_accepted);

            /* クライアントとの通信を確認するため、ビットマスクをセット */
            FD_SET(sock_accepted, &mask);
        } else {
            // 特定のユーザからメッセージが届いている
            member_t current = get_head_from_list();
            // 全メンバーのsockを確認する
            while (1) {
                int client_sock = current->sock;

                if (FD_ISSET(client_sock, &readfds)) {
                    // このユーザから受信する
                    strsize = Recv(client_sock, r_buf, MSGBUF_SIZE - 1, 0);
                    r_buf[strsize] = '\0';
                    packet = (ido_packet *)r_buf;
                    // 名前が未設定だったら
                    if (strlen(current->username) == 0) {
                        // 名前が設定されるまでここにくる
                        // パケットtypeはJOINである必要がある
                        if (analyze_header(packet->header) == JOIN) {
                            // 名前を設定する
                            snprintf(current->username, USERNAME_LEN, "%s", packet->data);
                        }
                        // JOIN以外は受け付けない
                    }
                    // 名前が設定されていたら
                    else {
                        switch (analyze_header(packet->header)) { /* ヘッダに応じて分岐 */
                            case HELLO:
                                /* HELOパケットを受けとった時の処理 */
                                // 何もしない
                                break;
                            case JOIN:
                                /* 以下、新規メンバー登録処理 */
                                // 何もしない
                                break;
                            case POST:
                                // 全員にメッセージを送信する
                                // send_message(int sock_from);
                                break;
                            case MESSAGE:
                                break;
                            case QUIT:
                                break;
                            default:
                                break;
                        }
                    }
                }

                if (current->next != NULL) {
                    // 次の場所がNULLではない、つまり次のノードがある。
                    // 現在の場所を次のノードに移動します。
                    current = current->next;
                } else {
                    // 次の場所がNULLなら、現在の場所はリストの末尾。
                    // もう次のデータはないので、breakでループ終了。
                    break;
                }
            }
        }
    }

    close(udp_sock);

    exit(EXIT_SUCCESS);
}

void send_message(char *message, char *username, int sock_from) {
    member_t current = get_head_from_list();

    char username_message[MSGBUF_SIZE];
    // ユーザ名を付加したメッセージを生成
    snprintf(username_message, MSGBUF_SIZE, "[%s]%s", username, message);

    // メッセージにヘッダをつける
    create_packet(MESSAGE, username_message);

    while (1) {
        // メッセージの送信者以外に送信する
        if (current->sock != sock_from) {
            Send(current->sock, Buffer, MSGBUF_SIZE, 0);
        }

        if (current->next != NULL) {
            // 次の場所がNULLではない、つまり次のノードがある。
            // 現在の場所を次のノードに移動します。
            current = current->next;
        } else {
            // 次の場所がNULLなら、現在の場所はリストの末尾。
            // もう次のデータはないので、breakでループ終了。
            break;
        }
    }
}

void show_adrsinfo(struct sockaddr_in *adrs_in) {
    int port_number;
    char ip_adrs[20];

    strncpy(ip_adrs, inet_ntoa(adrs_in->sin_addr), 20);
    port_number = ntohs(adrs_in->sin_port);

    printf("%s[%d]\n", ip_adrs, port_number);
}

unsigned int analyze_header(char *header) {
    if (strncmp(header, "HELO", 4) == 0) return (HELLO);
    if (strncmp(header, "HERE", 4) == 0) return (HERE);
    if (strncmp(header, "JOIN", 4) == 0) return (JOIN);
    if (strncmp(header, "POST", 4) == 0) return (POST);
    if (strncmp(header, "MESG", 4) == 0) return (MESSAGE);
    if (strncmp(header, "QUIT", 4) == 0) return (QUIT);
    return 0;
}

/* Max_sdの値は常に最大値を取るようにする*/
static void setMax_sd(int num) {
    if (num > Max_sd) {
        Max_sd = num;
    }
}
