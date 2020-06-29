/*
 * chat.c
*/
#include "chat.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mynet.h"

#define SERVER_LEN 256     /* サーバ名格納用バッファサイズ */
#define DEFAULT_PORT 50000 /* ポート番号既定値 */
#define DEFAULT_NCLIENT 3  /* 省略時のクライアント数 */
#define DEFAULT_MODE 'C'   /* 省略時はクライアント */

extern char *optarg;
extern int optind, opterr, optopt;

void help_message(char *script_name);

int main(int argc, char *argv[]) {
    int port_number = DEFAULT_PORT;
    int num_client = DEFAULT_NCLIENT;
    char servername[SERVER_LEN] = "localhost";
    char mode = DEFAULT_MODE;
    int c;

    /* オプション文字列の取得 */
    opterr = 0;
    while (1) {
        c = getopt(argc, argv, "SCs:p:c:h");
        if (c == -1)
            break;

        switch (c) {
            case 'S': /* サーバモードにする */
                mode = 'S';
                break;

            case 'C': /* クライアントモードにする */
                mode = 'C';
                break;

            case 's': /* サーバ名の指定 */
                snprintf(servername, SERVER_LEN, "%s", optarg);
                break;

            case 'p': /* ポート番号の指定 */
                port_number = atoi(optarg);
                break;

            case 'c': /* クライアントの数 */
                num_client = atoi(optarg);
                break;

            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optopt);
            case 'h':
                help_message(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    switch (mode) {
        case 'S':
            fprintf(stdout, "Mode: Server\n");
            fprintf(stdout, "Server Port: %d\n", port_number);
            fprintf(stdout, "Maximum number of clients: %d\n", num_client);
            chat_server(port_number, num_client);
            break;
        case 'C':
            fprintf(stdout, "Mode: Client\n");
            fprintf(stdout, "Server name: %s\n", servername);
            fprintf(stdout, "Server Port: %d\n", port_number);
            chat_client(servername, port_number);
            break;
    }

    exit(EXIT_SUCCESS);
}

void help_message(char *script_name) {
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "    %s [OPTIONS]\n\n", script_name);
    fprintf(stderr, "\n");

    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "    -S, \t\t Run Chat-Server mode\n");
    fprintf(stderr, "    -C, \t\t Run Chat-Client mode\n");
    fprintf(stderr, "    -p, <number> \t Set port number [default: 50000]\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "  for Server mode:\n");
    fprintf(stderr, "    -c, <number> \t Set maximum number of clients [default: 3]\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "  for Client mode:\n");
    fprintf(stderr, "    -s, <server_name> \t Set Chat-Sever address [default: localhost]\n");
}
