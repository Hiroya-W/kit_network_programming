# TCPサーバを初期化

backlog listenシステムコールの引数？

init_tcpserverはソケットを返す？



# TCPクライアントを初期化

ソケットの番号が返ってくる？



# ＃ifndef

大きなプロジェクトだとどれをインクルードしているかわからなくなる

インクルードしていなければ、エラーが表示されてコンパイルできないだけだが

2重にインクルードされる場合はめんどくさいことになる

ので、1度のみインクルードされるようにするためのもの

if not defineなら、という条件分岐となる



ヘッダファイルにはこうゆうのを書くのがお決まりらしい。



# アーカイブ

arコマンドを使うと.aという拡張子のファイルができる



# makeコマンド

タイムスタンプでコンパイルし直すべきファイルを自動で判断してくれる



# Eclipseによるライブラリの作成

Eclipseだと名前が自動的につく

lib + project name + .aというファイルが作成される



# ライブラリを使ったプログラムの作成

サーバに接続するのはinit_tcpclient関数を作ったので、これを使う

ライブラリを使えばシンプルになる



インクルードパスはシステムにパスを通しているものは特に指定はいらないが

自作ライブラリはシステムにパスを通しているわけがないので、

コンパイル時に指定しなくてはならない



## コンパイル時

ライブラリを別ディレクトリの中に作成したので、ディレクトリの指定をする必要がある

