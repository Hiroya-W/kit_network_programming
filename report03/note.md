# sockaddr_in

クライアントの場合は、サーバにアクセスするためのアドレスを指定したが

サーバは、自分自身のアドレスはOSレベルで分かっているので、明示的にIPアドレスを指定する必要はない

**INADDR_ANY**を用いる。



**htons**は、short型をネットワークバイトオーダへ、

**htonl**は、long型



# STREAMモード

待受をするソケット、送信するソケットの2種類ある

# アドレス情報を結びつける

クライアントからの情報を待ち受ける時に、どうゆう情報を持って待っているのか？

# クライアントの接続を受け付ける

acceptで、新しくソケットを使って、返す

その後、待受用に使っていたソケットを閉じる

他の受付を待っている様な場合は、閉じないけど。



# acceptの時？

既に受け付けていたら、すぐに次の処理を進めるけど

クライアントが接続していないのに、関数を呼ぶと、受付で止まったままの状態を維持すると思う

ある程度待って、接続してこなかったら別の動作、みたいなことは別の機会にやります



# echoプログラム

プログラム冒頭で指定したポート番号を今回のサーバと前回のクライアントで合わせる必要がある



サーバは50000にしたなら、クライアントもポートは50000にしないと行けないのか？

送信側のポートは1234、受信側は50000だったという。別にしないといけない。

クライアントのポート番号って何番でもいいよね。

だから、OSが適当に決めてくれるので、ユーザは気にしなくてもよい。

サーバはクライアントが接続してくるので、明示的に決めておかなくてはならない。



自分でテストする時は、一台のコンピュータで動かすことになると思う



# 小テスト

typoraでは、チェックをつけると線が引かれるので、チェックを

つけていないものが正解のものである。



TCPクライアントプログラミングに関する以下の文章のうち、正しいものをすべて選びなさい。

1つまたはそれ以上選択してください:

- [x] a. TCPで情報を送信するときはsend()、受信するときはreceive()をそれぞれ使う

- [ ] b. gethostbyname()システムコールは、DNSサーバを利用してホスト名からアドレス情報を得る ![正解](https://moodle.cis.kit.ac.jp/theme/image.php/clean/core/1586674684/i/grade_correct)

- [x] c. 1024番以下のポート番号に接続するTCPクライアントプログラムは管理者権限がないと実行できない

- [ ] d. 通常、ネットワークアプリケーションはソケットインタフェースを用いることによりOSのTCP/IPプロトコルスタックを利用できる ![正解](https://moodle.cis.kit.ac.jp/theme/image.php/clean/core/1586674684/i/grade_correct)

- [x] e. ネットワーク上に数値を送出するときは下位バイトから順に送信する。

- [x] f. socket()システムコールで、通信相手先のアドレス情報を指定してソケットを作成する

- [ ] g. TCPで通信を行なうときは、STREAMモードでソケットを作成する ![正解](https://moodle.cis.kit.ac.jp/theme/image.php/clean/core/1586674684/i/grade_correct)

- [ ] h. send()関数の引数に相手先のアドレス情報を指定する必要はない ![正解](https://moodle.cis.kit.ac.jp/theme/image.php/clean/core/1586674684/i/grade_correct)