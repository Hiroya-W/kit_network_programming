/*
 * Makefile
 * cryptを使用しているので、Makefileを変更している
 */

// CC = gcc
// MYLIBDIR =../mynet
// MYLIB =-lmynet -lcrypt
// CFLAGS =-I ${MYLIBDIR} -L ${MYLIBDIR} -W -Wall -g
//
// all: echo_server2
//
// echo_server2: echo_server2.o
// 	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}
//
// clean:
// 	${RM} *.o echo_server2 *~

#include <crypt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mynet.h"

#define R_BUFSIZE 2024 /* バッファサイズ */
#define S_BUFSIZE 1026 /* バッファサイズ */
#define CMD_BUFSIZE R_BUFSIZE + 100

const char *SEND_ERROR = "クライアントに送信できませんでした";
const char *UNKNOWN_COMMAND = "定義されていないコマンドです";
char *SET_PASS_MESSAGE = "パスワードを設定してください。\r\n新しいパスワード: ";
char *REGISTERED_PASS_MESSAGE = "パスワードを設定しました。再接続してください\r\n";
char *REQUIRE_PASS_MESSAGE = "パスワードを入力してください。\r\n";
char *PASS_PROMPT = "パスワード: ";
char *SUCCSESS_PASS_MESSAGE = "パスワード認証に成功しました\r\n";
char *FAILED_PASS_MESSAGE = "パスワード認証に失敗しました。入力しなおしてください。\r\n";
char *WORKSPACE_FOLDER = "~/Documents/";

void send_to_client(int sock_accepted, char *s_buf, int flag);
void get_op_code(char *out, char *r_buf);
void get_args(char *out, char *r_buf);
int path_find_filename(char *out, char *in);

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *argv[]) {
    /* TCP通信用 */
    int sock_listen, sock_accepted;
    int strsize;
    int port = 50000;
    char r_buf[R_BUFSIZE], s_buf[S_BUFSIZE];
    char op_code[R_BUFSIZE];
    int Continue = 1;

    /* パスワードハッシュ化 */
    char *salt = "$5$SALT";
    char hashed_pass[1024];

    /* オプション文字列の取得 */
    opterr = 0;
    int c = 0;
    while (1) {
        c = getopt(argc, argv, "p:h");
        if (c == -1) break;

        switch (c) {
            case 'p': /* ポート番号の指定 */
                port = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optopt);
            case 'h':
                fprintf(stderr, "Usage: %s -p port_number\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    sock_listen = init_tcpserver(port, 5);
    fprintf(stdout, "\nServer Port: %d\n\n", port);

    /* 1回目の接続ではパスワードを設定する */
    /* クライアントの接続を受け付ける */
    sock_accepted = accept(sock_listen, NULL, NULL);
    send_to_client(sock_accepted, SET_PASS_MESSAGE, 0);

    if ((strsize = recv(sock_accepted, r_buf, R_BUFSIZE, 0)) == -1) {
        fprintf(stderr, "recv()");
        exit(EXIT_FAILURE);
    }
    r_buf[strsize - 2] = '\0';
    strcpy(hashed_pass, crypt(r_buf, salt));

    fprintf(stdout, "ハッシュ化パスワード :%s\n", hashed_pass);
    send_to_client(sock_accepted, REGISTERED_PASS_MESSAGE, 0);

    close(sock_accepted);

    while (1) {
        /* クライアントの接続を受け付ける */
        sock_accepted = accept(sock_listen, NULL, NULL);
        Continue = 1;

        /* パスワード認証 */
        send_to_client(sock_accepted, REQUIRE_PASS_MESSAGE, 0);
        int auth = 0;

        while (!auth) {
            char *password;
            send_to_client(sock_accepted, PASS_PROMPT, 0);
            /* 文字列をクライアントから受信する */
            if ((strsize = recv(sock_accepted, r_buf, R_BUFSIZE, 0)) == -1) {
                fprintf(stderr, "recv()");
                exit(EXIT_FAILURE);
            }
            r_buf[strsize - 2] = '\0';
            password = crypt(r_buf, salt);

            fprintf(stdout, "入力パスワードのハッシュ %s\n", password);
            if (strcmp(hashed_pass, password) == 0) {
                send_to_client(sock_accepted, SUCCSESS_PASS_MESSAGE, 0);
                auth = 1;
            } else {
                send_to_client(sock_accepted, FAILED_PASS_MESSAGE, 0);
            }
        }

        while (Continue) {
            send_to_client(sock_accepted, "> ", 0);
            if ((strsize = recv(sock_accepted, r_buf, R_BUFSIZE, 0)) == -1) {
                fprintf(stderr, "recv()");
                exit(EXIT_FAILURE);
            }
            /* ゴミを受け取る */
            char temp[R_BUFSIZE];
            strcpy(temp, r_buf);
            while (temp[strsize - 1] != '\n') {
                if ((strsize = recv(sock_accepted, temp, R_BUFSIZE, 0)) == -1) {
                    fprintf(stderr, "recv()");
                    exit(EXIT_FAILURE);
                }
            }

            strsize = strlen(r_buf);
            r_buf[strsize] = '\0';

            get_op_code(op_code, r_buf);
            if (strcmp(op_code, "exit") == 0) {
                Continue = 0;
                break;
            } else if (strcmp(op_code, "list") == 0) {
                FILE *fp;
                char *prefix = "/usr/bin/ls";
                char cmd[CMD_BUFSIZE];
                snprintf(cmd, CMD_BUFSIZE, "%s %s", prefix, WORKSPACE_FOLDER);

                if ((fp = popen(cmd, "r")) != NULL) {
                    while (fgets(s_buf, S_BUFSIZE, fp) != NULL) {
                        send_to_client(sock_accepted, s_buf, 0);
                        fprintf(stdout, "%s", s_buf);
                    }
                    pclose(fp);
                }
                send_to_client(sock_accepted, "\r\n", 0);
            } else if (strcmp(op_code, "type") == 0) {
                get_args(op_code, r_buf);
                /* ディレクトリトラバーサル対策 */
                path_find_filename(op_code, op_code);

                FILE *fp;
                char *prefix = "/usr/bin/cat";
                char cmd[CMD_BUFSIZE];
                snprintf(cmd, CMD_BUFSIZE, "%s %s%s", prefix, WORKSPACE_FOLDER, op_code);
                if ((fp = popen(cmd, "r")) != NULL) {
                    while (fgets(s_buf, S_BUFSIZE, fp) != NULL) {
                        send_to_client(sock_accepted, s_buf, 0);
                        fprintf(stdout, "%s", s_buf);
                    }
                }
                pclose(fp);
                send_to_client(sock_accepted, "\r\n", 0);
            } else {
                fprintf(stderr, "%s\n", UNKNOWN_COMMAND);
                snprintf(s_buf, S_BUFSIZE, "%s\r\n", UNKNOWN_COMMAND);
                send_to_client(sock_accepted, s_buf, 0);
            }
        }
        close(sock_accepted);
    }
    /* Unreachable */
    close(sock_listen);

    exit(EXIT_SUCCESS);
}

void send_to_client(int sock_accepted, char *s_buf, int flag) {
    /* 文字列をクライアントに送信する */
    int strsize = strlen(s_buf);
    if (send(sock_accepted, s_buf, strsize, flag) == -1) {
        fprintf(stderr, "%s\n", SEND_ERROR);
        exit(EXIT_FAILURE);
    }
    return;
}

void get_op_code(char *out, char *r_buf) {
    char *p;
    long len = 0;

    p = strstr(r_buf, "\r\n");
    if (!(p == NULL)) {
        *p = '\0';
    }
    strcpy(out, r_buf);

    p = strstr(r_buf, " ");
    if (!(p == NULL)) {
        strcpy(out, r_buf);
        len = p - r_buf;
        out[len] = '\0';
        strcpy(r_buf, p + 1);
    }

    return;
}

void get_args(char *out, char *r_buf) {
    char *p;
    long len = 0;

    if (*r_buf != '\0') {
        p = strstr(r_buf, " ");
        if (p == NULL) {
            strcpy(out, r_buf);
        } else {
            len = p - r_buf;
            r_buf[len] = '\0';
            strncpy(out, r_buf, len);
        }
    }

    return;
}

int path_find_filename(char *out, char *in) {
    const char pattern[] = "([^/]+$)";
    regex_t regexBuffer;
    regmatch_t match[1];
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
        strncpy(out, in + startIndex, endIndex - startIndex);
        out[endIndex - startIndex] = '\0';
        // printf("%s\n", out[i]);
    }
    regfree(&regexBuffer);
    return 0;
}
