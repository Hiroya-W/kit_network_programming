/*
 * idobata.c
 */
#include "idobata.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mynet.h"

#define DEFAULT_PORT 50001 /* ポート番号既定値 */

extern char *optarg;
extern int optind, opterr, optopt;

void help_message(char *script_name);

int main(int argc, char *argv[]) {
    int port_number = DEFAULT_PORT;
    char user_name[USERNAME_LEN];
    int is_name_changed = 0;
    int c;

    /* オプション文字列の取得 */
    opterr = 0;
    while (1) {
        c = getopt(argc, argv, "n:p:h");
        if (c == -1)
            break;
        switch (c) {
            case 'n': /* ユーザ名の指定 */
                snprintf(user_name, USERNAME_LEN, "%s", optarg);
                is_name_changed = 1;
                break;
            case 'p': /* ポート番号の指定 */
                port_number = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optopt);
            case 'h':
                help_message(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }
    /* 名前入力されていなければ入力させる */
    if (!is_name_changed) {
        fprintf(stderr, "Please set user name by using -n option.\n");
        help_message(argv[0]);
        exit(EXIT_FAILURE);
    }
    if (search_server(port_number)) {
    } else {
        idobata_server(port_number);
    }

    exit(EXIT_SUCCESS);
}

/* helpを表示する */
void help_message(char *script_name) {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "    %s [OPTIONS]\n\n", script_name);
    fprintf(stderr, "\n");

    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr,
            "    -p, <number> \t Set (TCP/UDP)port number [default: 50001]\n");
    fprintf(stderr, "    -n, <user_name> \t Set user name\n");
    fprintf(stderr, "    -h,  \t\t Print this help message\n");
}
