# c-compiler

[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)

## Flow

コード文字列 -> トークン列 -> 抽象構文木 -> アセンブリ

## メモリ管理について
* 解放はしない
* Cファイルを読み込んでアセンブリを出力するだけの短命なプログラムゆえ、プログラム終了時に確保されているメモリをOSが自動解放する機能に任せる

## 用語

### 文法の記述方法と再帰下降構文解析
* トークン
    * ある言語における単語、基本単位
* トークナイズ
    * 文字列をトークン列に分割すること
* 構文木
    * トークンの入れ子構造を木で表現したもの
    * 評価を葉ノードの左から行うことで、計算順序を表現できる
* 抽象構文木
    * 構文木から冗長な要素(カッコなど)を除いたもの
* パーサ
    * トークン列を抽象構文木に変換する
* 生成規則
    * 文法を再帰的に定義するルール
    * 再帰によって入れ子構造を表現できる
* BNF
    * Backus-Naur form
    * 各生成規則を`A = α1α2...`という形式で表現
        * `A`を`α1α2...`に展開できることを表す
    * 左辺には非終端記号(さらに展開できる記号)が含まれ、右辺には終端記号(これ以上展開できない記号)と非終端記号の少なくとも一方が含まれる
    * 非終端記号は複数の生成規則にマッチしてよい
    * ε(イプシロン)
        * 長さ0の記号列を便宜的に表現したもの
    * 文脈自由文法
        * BNFのような生成規則で定義される文法
* EBNF
    * Extended BNF
    * BNFの記法を簡略化したもの
* 再帰下降構文解析
    * 1つの生成規則を1つの関数にマップする構文解析手法
    * LL(1)パーサ
        * トークンを1つだけ先読みして、どの関数を呼び出すかを決めるパーサ

### スタックマシン
* スタックをデータ保存領域として持っているコンピュータ
* どのような部分式も、それを実行した結果として1つの要素をスタックに結果として残すという約束を守る限り、うまくコンパイルできる
* 抽象構文木をスタックマシンにコンパイルするときは、再帰的に考えて、木を葉から根に下りながらどんどんアセンブリを出力していく

### 分割コンパイルとリンク
* オブジェクトファイル
    * 単体では実行不可能なプログラムの断片が入ったファイル
* リンカ
    * オブジェクトファイルをまとめて1つの実行ファイルにするプログラム