/*
   Proxyを介してHTTPサーバと対話するクライアントプログラム
　　（利用者が自らHTTPを入力する必要がある）
*/
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// #define PROXYPORT 8080  /* プロキシサーバのポート番号 */
#define PORT 80
#define BUFSIZE 1024 /* バッファサイズ */

int get_cont_by_fieldname(char out[], char buf[], char word[]);

int main() {
    struct hostent *server_host;
    // struct sockaddr_in proxy_adrs;
    struct sockaddr_in server_adrs;

    int tcpsock;

    // char proxyname[] = "proxy.cis.kit.ac.jp"; /* プロキシサーバ */
    // http://www.sec.is.kit.ac.jp/index.html
    char servername[] = "www.sec.is.kit.ac.jp";
    char k_buf[BUFSIZE], s_buf[BUFSIZE], r_buf[BUFSIZE];
    int strsize;

    /* サーバ名をアドレス(hostent構造体)に変換する */
    if ((server_host = gethostbyname(servername)) == NULL) {
        fprintf(stderr, "gethostbyname()");
        exit(EXIT_FAILURE);
    }

    /* サーバの情報をsockaddr_in構造体に格納する */
    memset(&server_adrs, 0, sizeof(server_adrs));
    server_adrs.sin_family = AF_INET;
    server_adrs.sin_port = htons(PORT);
    memcpy(&server_adrs.sin_addr, server_host->h_addr_list[0], server_host->h_length);

    /* ソケットをSTREAMモードで作成する */
    if ((tcpsock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "socket()");
        exit(EXIT_FAILURE);
    }

    /* ソケットにサーバの情報を対応づけてサーバに接続する */
    if (connect(tcpsock, (struct sockaddr *)&server_adrs, sizeof(server_adrs)) == -1) {
        fprintf(stderr, "connect");
        exit(EXIT_FAILURE);
    }

    /* キーボードから文字列を入力してサーバに送信 */
    while ((fgets(k_buf, BUFSIZE, stdin) != NULL) && k_buf[0] != '\n') { /* 空行が入力されるまで繰り返し */
        strsize = strlen(k_buf);
        k_buf[strsize - 1] = 0;                    /* 末尾の改行コードを消す */
        snprintf(s_buf, BUFSIZE, "%s\r\n", k_buf); /* HTTPの改行コードは \r\n */

        /* 文字列をサーバに送信する */
        if (send(tcpsock, s_buf, strsize + 1, 0) == -1) {
            fprintf(stderr, "send()");
            exit(EXIT_FAILURE);
        }
    }

    send(tcpsock, "\r\n", 2, 0); /* HTTPのメソッド（コマンド）の終わりは空行 */

    /* サーバから文字列を受信する */
    if ((strsize = recv(tcpsock, r_buf, BUFSIZE - 1, 0)) == -1) {
        fprintf(stderr, "recv()");
        exit(EXIT_FAILURE);
    }
    r_buf[strsize] = '\0';

    /* 受信した文字列を画面に書く */
    printf("%s", r_buf);

    /* fieldの表示 */
    printf("\n");
    char *p;
    p = strstr(r_buf, "Server");
    if (p != NULL) {
        while (*p != '\n') {
            printf("%c", *p);
            p++;
        }
        printf("\n");
    } else {
        printf("Serve fieldが見つかりませんでした\n");
    }

    char contents_of_server[BUFSIZE] = {};
    if (get_cont_by_fieldname(contents_of_server, r_buf, "Server") != -1) {
        printf("サーバープログラム: %s\n", contents_of_server);
    } else {
        printf("Serverフィールドは含まれていません。\n");
    }

    char size_of_contents[BUFSIZE] = {};
    if (get_cont_by_fieldname(size_of_contents, r_buf, "Content-Length") != -1) {
        printf("コンテンツの大きさ: %s\n", size_of_contents);
    } else {
        printf("Content-Lengthフィールドは含まれていません。\n");
    }

    close(tcpsock); /* ソケットを閉じる */
    exit(EXIT_SUCCESS);
}

int get_cont_by_fieldname(char out[], char buf[], char word[]) {
    char *p = NULL;
    int len = 0;
    p = strstr(buf, word);

    if (p == NULL) {
        return -1;
    }

    p = strstr(p, ": ") + 2;
    len = strstr(p, "\r\n") - p;
    len = len < BUFSIZE ? len : BUFSIZE;

    strncpy(out, p, len);
    out[len] = '\0';
    return 0;
}
