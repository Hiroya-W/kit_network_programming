
# 第8回

タイプ: ノート
作成日: Jun 22, 2020 1:06 PM
復習済み: No
講義名: ネットワークプログラミング

Moodleのようなサーバを考えてみる

関数間で情報を受け渡す方法として、関数の引数を使う方法と、大域変数を利用する方法

外部変数が一番広いけど、問題が起きるので、使わない

広すぎても問題があるのよね

# quiz_util.c

構造体を用意してる

staticで宣言している変数がある

このファイルの外で定義されている関数からこれらの変数を参照することができない

どうように、関数も外から呼ぶことができない

名前が被ってしまって動作がおかしい、みたいなことを防ぐ

配列のサイズはコンパイル時に決まっていないといけない

ので、代わりにmallocを使って確保する

ゆーざ情報を保存するのは

Client[client_id.sock = sock_accepted

すればいい

selectで一番大きなsockの番号が必要になるので、returnしておく

# ヘッダファイル

staticが付いている関数は記述したところで、呼び出せないので

（ほんまか？）

staticついてないものだけ書く
