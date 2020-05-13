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

#define BUFSIZE 1024 /* バッファサイズ */

int get_cont_by_fieldname(char out[], char buf[], char word[]);
int url_parse(char out[3][128], char in[]);

int main(int argc, char *argv[]) {
    struct hostent *server_host;
    struct sockaddr_in server_adrs;

    int tcpsock;
    int port = 80;
    int strsize;

    char url[3][128] = {};
    char servername[128] = {};
    char hostname[128] = {};
    char k_buf[BUFSIZE], s_buf[BUFSIZE], r_buf[BUFSIZE];

    if (argc == 2) {
        if (url_parse(url, argv[1]) == -1) {
            fprintf(stderr, "URLのパースに失敗しました。\n");
            return 1;
        } else if (strcmp(url[1], "http") != 0) {
            fprintf(stderr, "HTTPプロトコルにのみ対応しています。\n");
            return 1;
        }
        strcpy(hostname, url[2]);
    } else if (argc == 4) {
        strcpy(hostname, argv[2]);
        port = atoi(argv[3]);
    } else {
        fprintf(stderr, "使い方: http://example.com [proxy.host.name port]\n");
        return 1;
    }

    fprintf(stdout, "HTTPリクエストを送信して、サーバプログラムとコンテンツのサイズを取得します。\n");
    fprintf(stdout, "HEAD %s HTTP/1.1\n", argv[1]);
    fprintf(stdout, "Host: %s\n\n", hostname);

    /* サーバ名をアドレス(hostent構造体)に変換する */
    if ((server_host = gethostbyname(hostname)) == NULL) {
        fprintf(stderr, "サーバ名をアドレスに変換できませんでした。\n");
        exit(EXIT_FAILURE);
    }

    /* サーバの情報をsockaddr_in構造体に格納する */
    memset(&server_adrs, 0, sizeof(server_adrs));
    server_adrs.sin_family = AF_INET;
    server_adrs.sin_port = htons(port);
    memcpy(&server_adrs.sin_addr, server_host->h_addr_list[0], server_host->h_length);

    /* ソケットをSTREAMモードで作成する */
    if ((tcpsock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "ソケットの作成ができませんでした。\n");
        exit(EXIT_FAILURE);
    }

    /* ソケットにサーバの情報を対応づけてサーバに接続する */
    if (connect(tcpsock, (struct sockaddr *)&server_adrs, sizeof(server_adrs)) == -1) {
        fprintf(stderr, "サーバに接続できませんでした。\n");
        exit(EXIT_FAILURE);
    }

    /* 文字列をサーバに送信する */
    /**
     * HEAD http://example.com
     */
    snprintf(s_buf, BUFSIZE, "HEAD %s HTTP/1.1\r\n", argv[1]);
    strsize = strlen(s_buf);
    if (send(tcpsock, s_buf, strsize, 0) == -1) {
        fprintf(stderr, "文字列をサーバに送信できませんでした。\n");
        exit(EXIT_FAILURE);
    }
    /**
     * Host: example.com
     */
    snprintf(s_buf, BUFSIZE, "Host: %s\r\n", hostname);
    strsize = strlen(s_buf);
    if (send(tcpsock, s_buf, strsize, 0) == -1) {
        fprintf(stderr, "文字列をサーバに送信できませんでした。\n");
        exit(EXIT_FAILURE);
    }

    /**
     * 改行
     */
    send(tcpsock, "\r\n", 2, 0); /* HTTPのメソッド（コマンド）の終わりは空行 */

    /* サーバから文字列を受信する */
    if ((strsize = recv(tcpsock, r_buf, BUFSIZE - 1, 0)) == -1) {
        fprintf(stderr, "サーバから文字列を受信できませんでした。\n");
        exit(EXIT_FAILURE);
    }
    r_buf[strsize] = '\0';

    /** 
     * fieldの内容の表示
     */
    char contents_of_server[BUFSIZE] = {};
    if (get_cont_by_fieldname(contents_of_server, r_buf, "Server") != -1) {
        printf("サーバープログラム: %s\n", contents_of_server);
    } else {
        fprintf(stderr, "Serverフィールドは含まれていません。\n");
    }

    char size_of_contents[BUFSIZE] = {};
    if (get_cont_by_fieldname(size_of_contents, r_buf, "Content-Length") != -1) {
        printf("コンテンツの大きさ: %s\n", size_of_contents);
    } else {
        fprintf(stderr, "Content-Lengthフィールドは含まれていません。\n");
    }

    close(tcpsock); /* ソケットを閉じる */
    exit(EXIT_SUCCESS);
}

/**
 * @fun
 * 入力されたURLをパースして、SchemeとHostを取得する。 
 * @param (out) パースした結果として、URL Scheme Hostを格納する配列
 * @param (in) URL文字列が格納された配列
 * @return 成功した時:0 失敗した時:-1
 */
int url_parse(char out[3][128], char in[]) {
    const char pattern[] = "^([httpsfile]+)://([0-9a-zA-Z.-]+)/?";
    regex_t regexBuffer;
    regmatch_t match[3];
    int size;

    // 正規表現オブジェクトをコンパイル
    if (regcomp(&regexBuffer, pattern, REG_EXTENDED | REG_NEWLINE) != 0) {
        printf("正規表現が不正です。\n");
        return -1;
    }
    // 正規表現パターンとマッチする？
    size = sizeof(match) / sizeof(regmatch_t);
    if (regexec(&regexBuffer, in, size, match, 0) != 0) {
        printf("正規表現パターンにURLがマッチしませんでした。\n");
        return -1;
    }
    // パターンマッチした場合の処理
    int i;
    for (i = 0; i < size; i++) {
        // マッチした位置の開始・終了インデックス
        int startIndex = match[i].rm_so;  // 開始インデックス
        int endIndex = match[i].rm_eo;    // 終了インデックス
        if (startIndex == -1 || endIndex == -1) {
            // printf("exit");
            continue;
        }
        // printf("index [start, end] = %d, %d\n", startIndex, endIndex);
        strncpy(out[i], in + startIndex, endIndex - startIndex);
        out[i][endIndex - startIndex] = '\0';
        // printf("%s\n", out[i]);
    }
    regfree(&regexBuffer);
    return 0;
}

/**
 * @fun
 * HTTPレスポンスのヘッダフィールドから指定したフィールド名の内容を取得する。
 * @param (out) 指定したフィールド名の内容を格納する配列
 * @param (buf) HTTPレスポンスのヘッダフィールド全文を入力とする
 * @return 成功した時:0 失敗した時:-1
 */
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
