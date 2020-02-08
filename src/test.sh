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
#echo "### step4: エラーメッセージを改良"
#try 1 "1+3++"
#try 1 "1 + foo + 3"
echo ok