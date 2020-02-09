#!/bin/bash

try () {
    local -r expected="$1"
    local -r input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    local -r actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

echo "### step1: 整数1個をコンパイルする言語の作成"
try 0 0
try 42 42

echo "### step2: 加減算のできるコンパイラの作成"
try 21 "5+20-4"

echo "### step3: トークナイザを導入"
try 41 " 12 + 34 - 5 "

echo "### step5: 四則演算のできる言語の作成"
try 47 "5+6*7"
try 15 "5*(9-6)"
try 4 "(3 + 5) / 2"

echo "### step6: 単項プラスと単項マイナス"
try 10 "-10+20"
try 10 "- -10"
try 10 "- - +10"

echo "### step7: 比較演算子"
try 1 "1>0"
try 0 "0>1"
try 0 "1>2"
try 1 "1>=0"
try 1 "1>=1"
try 0 "1>=2"

echo "### step8: 分割コンパイルとリンク"


echo ok