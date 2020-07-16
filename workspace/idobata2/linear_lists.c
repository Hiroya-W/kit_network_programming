#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "idobata.h"

member_t head = NULL;
member_t tail = NULL;

member_t create_member();
member_t get_head_from_list();
void add_user_to_list(char *username, int sock);
void delete_user_from_list(int sock);
void print_list();

int test() {
    add_user_to_list("HIRO\0", 1);
    add_user_to_list("RYO\0", 2);
    add_user_to_list("MAHO\0", 3);
    print_list();
    delete_user_from_list(1);
    print_list();
    delete_user_from_list(2);
    print_list();
    delete_user_from_list(3);
    print_list();
    delete_user_from_list(3);
    print_list();
    return 0;
}

member_t get_head_from_list() {
    return head;
}

/* 線形リストの実装
 * https://www.mafuyu7se.com/entry/2019/01/14/184920 
 */

/* 新しくメンバーを生成する関数 */
member_t create_member() {
    // メンバー1人分のメモリを確保する
    member_t new_member;
    new_member = (member_t)malloc(sizeof(struct _imember));

    return new_member;
}

/* 新しいメンバーを線形リストの末尾に追加する */
void add_user_to_list(char *username, int sock) {
    member_t new_member;
    // 新しいメンバーを生成
    new_member = create_member();
    // メンバーの情報を格納
    snprintf(new_member->username, USERNAME_LEN, "%s", username);
    new_member->sock = sock;

    if ((head == NULL) && (tail == NULL)) {
        // リストが空の場合はこちら。
        // 新しいノードが、先頭かつ末尾になる。
        head = new_member;
        tail = new_member;
    } else {
        // リストに1件以上ノードが存在する場合はこちら。
        // 末尾ノード(tail)のnextに、新しいノードの場所をセット。
        tail->next = new_member;
        // 追加した新しいノードをtailとする。
        tail = new_member;
    }
    // 次のデータは無いので、nextにはNULLをセット。
    tail->next = NULL;
    // fprintf(stdout, "新しいメンバー %s を追加しました。\n", username);
}

/* リストを表示する */
void print_list() {
    member_t current;
    if ((head == NULL) && (tail == NULL)) {
        // 表示するものがない場合は何もせずにreturn。
        fprintf(stdout, "リストは空です。\n");
        return;
    }

    // 先頭から表示するので、現在の位置をheadにセット。
    current = head;
    fprintf(stdout, "----------------------------------\n");

    while (1) {
        fprintf(stdout, "sock : %d\n", current->sock);
        fprintf(stdout, "username     : %s\n", current->username);
        fprintf(stdout, "自身のノードの場所 : %ld\n", (long)current);
        fprintf(stdout, "次のノードの場所   : %ld\n", (long)current->next);
        fprintf(stdout, "----------------------------------\n");
        if (current->next != NULL) {
            // 次の場所がNULLではない、つまり次のノードがある。
            // 現在の場所を次のノードに移動します。
            current = current->next;
        } else {
            // 次の場所がNULLなら、現在の場所はリストの末尾。
            // もう次のデータはないので、breakでループ終了。
            fprintf(stdout, "最後まで表示しました。\n");
            break;
        }
    }
}

/* sockを指定して、そのメンバーを削除する */
void delete_user_from_list(int sock) {
    member_t current, prev;
    // リストの先頭から、sockが一致するノードを探す。
    current = head;
    prev = NULL;

    while (current != NULL) {
        if (current->sock == sock) {
            // ノードが見つかればループ終了。
            // fprintf(stdout, "削除するノードが見つかりました。\n");
            break;
        } else {
            // 現在のノードを1つ前のノード(prev)として記憶。
            prev = current;
            current = current->next;
        }
    }
    // 削除するノードが見つからなかったら、何もせずreturn。
    if (current == NULL) {
        // fprintf(stdout, "ノードが見つかりませんでした。\n");
        return;
    }

    if (head == tail) {
        // 削除対象が先頭かつ末尾のノードだった時。
        // リストが空になるので、NULLをセットする。
        free(current);
        head = NULL;
        tail = NULL;
    } else if (current == head) {
        // 削除対象が先頭のノードだった場合。
        // 先頭の次のノードを新しいheadとする。
        head = current->next;
        // 削除対象のノードを開放。
        free(current);
    } else if (current == tail) {
        // 削除対象が末尾のノードだった場合
        // 末尾より一個前をtailにする
        tail = prev;
        free(current);
    } else {
        // 削除対象が先頭のノードではない場合。
        // 「削除対象の1つ前」のnextに、「削除対象の1つ後ろ」をセット。
        prev->next = current->next;
        // 削除対象のノードを開放。
        free(current);
    }
    // fprintf(stdout, "削除が完了しました。\n");
}
