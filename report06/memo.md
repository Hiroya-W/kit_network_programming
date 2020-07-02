
# 第6回コマンドメモ

タイプ: メモ
作成日: Jun 11, 2020 3:16 PM
復習済み: No
講義名: ネットワークプログラミング

# netstat

netstatコマンドを 利用して、サーバプロセス（および、forkにより作成されたプロセス）やクライアント プロセスが利用しているIPアドレスやポート番号を調べてみましょう。 複数のクライアントが接続している状態で、それらがどうなっているか 確認しましょう。

```bash
netstat -nap | grep echo_fork
```

- -a	全てのアクティブなソケットを表示する
- -n	ホストやユーザーの名前解決を行わず数字のまま出力する
- -p	ソケットが属すプログラムのPIDとプロセス名を表示する

```bash
(*'-') < netstat -nap | grep echo_fork
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
tcp        0      0 0.0.0.0:50000           0.0.0.0:*               LISTEN      10103/./echo_fork
tcp        0      0 127.0.0.1:50000         127.0.0.1:40578         ESTABLISHED 12201/./echo_fork
tcp        0      0 127.0.0.1:50000         127.0.0.1:40582         ESTABLISHED 12257/./echo_fork
```

# procs

A modern replacement for ps written in Rust

動作中のプロセスの様子は、psコマンドを用いて確認することができます。

```bash
procs echo_fork --watch --tree
```

- キーワード "echo_fork"を含むもののみ表示する
- —watch 1秒インターバルで更新する
- —tree tree表示にする

```bash
(*'-') < ps echo_fork --tree
        PID   User   │ State Nice TTY   CPU MEM  VmPeak  VmSize   VmRSS TCP     UDP  Read Write │ CPU Time Start
                     │                  [%] [%] [bytes] [bytes] [bytes]             [B/s] [B/s] │
 └┬──── 10103 hiroya │ S        0 pts/3 0.0 0.0  2.711M  2.500M  1.555M [50000] []  0.000 0.000 │ 00:00:00 2020/06/11
  ├──── 12201 hiroya │ S        0 pts/3 0.0 0.0  2.500M  2.500M 92.000K []      []  0.000 0.000 │ 00:00:00 2020/06/11
  └──── 12257 hiroya │ S        0 pts/3 0.0 0.0  2.500M  2.500M 92.000K []      []  0.000 0.000 │ 00:00:00 2020/06/11
```

1つ、telnetで文字を送信して終了させてみると、

```bash
(*'-') < ps echo_fork --tree
        PID   User   │ State Nice TTY   CPU MEM  VmPeak  VmSize   VmRSS TCP     UDP  Read Write │ CPU Time Start
                     │                  [%] [%] [bytes] [bytes] [bytes]             [B/s] [B/s] │
 └┬──── 10103 hiroya │ S        0 pts/3 0.0 0.0  2.711M  2.500M  1.555M [50000] []  0.000 0.000 │ 00:00:00 2020/06/11
  ├──── 12201 hiroya │ S        0 pts/3 0.0 0.0  2.500M  2.500M 92.000K []      []  0.000 0.000 │ 00:00:00 2020/06/11
  └──── 12257 hiroya │ Z        0 pts/3 0.0 0.0           0.000   0.000                         │ 00:00:00 2020/06/11
```

stateがZでZombiになってる？

ppidを表示するには—insert ppidする

```bash
(*'-') < ps echo_fork --tree --insert ppid
        PID   User   │ State Nice TTY   CPU MEM  VmPeak  VmSize    VmRSS TCP     UDP  Read Write Parent PID │ CPU Tim
                     │                  [%] [%] [bytes] [bytes]  [bytes]             [B/s] [B/s]            │
 └┬──── 16294 hiroya │ S        0 pts/2 0.0 0.0  2.711M  2.500M 588.000K [50001] []  0.000 0.000       7197 │ 00:00:0
  ├──── 16295 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
  ├──── 16296 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
  ├──── 16297 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
  ├──── 16298 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
  ├──── 16299 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
  ├──── 16300 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
  ├──── 16301 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
  ├──── 16302 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
  └──── 16303 hiroya │ S        0 pts/2 0.0 0.0  2.500M  2.500M  92.000K [50001] []  0.000 0.000      16294 │ 00:00:0
```

このときの出力はこんな感じ

```bash
(*;-;) < ./echo_fork -p 50001 -r 10
終了する時はCtrl + Cする
Server Port: 50001
Process Limit: 10

Parent ppid[7197],pid[16294]
Client is accepted.ppid[16294],pid[16295]
Client is accepted.ppid[16294],pid[16298]
Client is accepted.ppid[16294],pid[16297]
Client is accepted.ppid[16294],pid[16299]
Client is accepted.ppid[16294],pid[16300]
Client is accepted.ppid[16294],pid[16302]
Client is accepted.ppid[16294],pid[16296]
Client is accepted.ppid[16294],pid[16301]
Client is accepted.ppid[16294],pid[16303]
```
