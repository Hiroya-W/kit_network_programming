/*
     Proxyを介してHTTPサーバと対話するクライアントプログラム
　　（利用者が自らHTTPを入力する必要がある）
*/
#include <netdb.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
// #define PROXYPORT 8080    /* プロキシサーバのポート番号 */
// #define SERVERPORT 80
#define BUFSIZE 1024 /* バッファサイズ */

int get_content(char out[], char buf[], char key[]) {
    char *p = NULL;
    // 文字列検索
    p = strstr(buf, key);
    if (p == NULL) {
        return -1;
    }
    printf("p=%s\n", p);
}

int main(int argc, char *argv[]) {
    struct hostent *server_host;
    struct sockaddr_in server_adrs;
    printf("%s", argv[1]);
    int tcpsock;
    int port = 80;
    int strsize;

    char url[3][128] = {};
    char servername[128] = {};
    char hostname[128] = {};
    char k_buf[BUFSIZE], s_buf[BUFSIZE], r_buf[BUFSIZE];

    // int tcpsock;
    if (argc < 2) {
        puts("URL を入力して下さい");
        return 0;
    }
    // プロキシなし
    if (argc == 2) {
        // puts("aa");
        printf("%s", argv[1]);
        strcpy(servername, argv[1]);
        puts("aa");
        printf("%s", servername);
        port = atoi(argv[2]);
    }
    // プロキシあり
    if (argc == 4) {
        strcpy(hostname, argv[1]);
        port = atoi(argv[2]);
    }
    fprintf(stdout, "HEAD %s HTTP/1.1\n", argv[1]);

    //     char proxyname[] = "proxy.cis.kit.ac.jp"; /* プロキシサーバ */
    // char servername[] = "www.is.kit.ac.jp";

    /* サーバ名をアドレス(hostent構造体)に変換する */
    if ((server_host = gethostbyname(servername)) == NULL) {
        fprintf(stderr, "サーバ名をアドレスに変換出来ませんでした");
        exit(EXIT_FAILURE);
    }
    /* サーバの情報をsockaddr_in構造体に格納する */
    memset(&server_adrs, 0, sizeof(&server_adrs));
    server_adrs.sin_family = AF_INET;
    server_adrs.sin_port = htons(port);
    memcpy(&server_adrs.sin_addr, server_host->h_addr_list[0], server_host->h_length);

    /* ソケットをSTREAMモードで作成する */
    if ((tcpsock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "ソケット作成に失敗しました");
        exit(EXIT_FAILURE);
    }

    /* ソケットにサーバの情報を対応づけてサーバに接続する */
    if (connect(tcpsock, (struct sockaddr *)&server_adrs, sizeof(server_adrs)) == -1) {
        fprintf(stderr, "サーバに接続できませんでした");
        exit(EXIT_FAILURE);
    }

    // HEAD
    snprintf(s_buf, BUFSIZE, "HEAD %s HTTP/1.1\r\n", argv[1]);
    if (send(tcpsock, s_buf, strlen(s_buf), 0) == -1) {
        fprintf(stderr, "HEAD: 文字列のサーバ送信に失敗しました\n");
        exit(EXIT_FAILURE);
    }

    // HOST
    snprintf(s_buf, BUFSIZE, "Host: %s\r\n", servername);
    // strsize = strlen(s_buf);
    if (send(tcpsock, s_buf, strlen(s_buf), 0) == -1) {
        fprintf(stderr, "HOST: 文字列のサーバ送信に失敗しました\n");
        exit(EXIT_FAILURE);
    }

    // 改行
    send(tcpsock, "\r\n", 2, 0);

    // 受信
    if ((strsize = recv(tcpsock, r_buf, BUFSIZE - 1, 0)) == -1) {
        fprintf(stderr, "サーバから文字列を受信できませんでした。\n");
        exit(EXIT_FAILURE);
    }
    r_buf[strsize] = '\0';
    char contents[BUFSIZE] = {};
    get_content(contents, r_buf, "Server");
    // if (get_cont_by_fieldname(contents, r_buf, "Server") != -1) {
    //     printf("サーバープログラム: %s\n", contents);
    // } else {
    //     fprintf(stderr, "Serverフィールドは含まれていません。\n");
    // }

    close(tcpsock); /* ソケットを閉じる */
    exit(EXIT_SUCCESS);
}
