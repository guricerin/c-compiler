#!/bin/bash

readonly base_path=$(dirname ${0} | pwd)

try () {
    local -r expected="$1"
    local -r input="$2"
    local -r assembly="${base_path}/obj/tmp.s"
    local -r binary="${base_path}/obj/tmp"

    ${base_path}/bin/9cc "$input" > "$assembly"
    gcc -static -o "$binary" "$assembly"
    $binary
    local -r actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

echo "### step11: return"
echo "### step1: 整数1個をコンパイルする言語の作成"
try 0 'return 0;'
try 42 'return 42;'
echo "### step2: 加減算のできるコンパイラの作成"
try 21 'return 5+20-4;'
echo "### step3: トークナイザを導入"
try 41 'return  12 + 34 - 5 ;'
echo "### step5: 四則演算のできる言語の作成"
try 47 'return 5+6*7;'
try 15 'return 5*(9-6);'
try 4 'return (3+5)/2;'
echo "### step6: 単項プラスと単項マイナス"
try 10 'return -10+20;'
try 10 'return - -10;'
try 10 'return - - +10;'
echo "### step7: 比較演算子"
try 0 'return 0==1;'
try 1 'return 42==42;'
try 1 'return 0!=1;'
try 0 'return 42!=42;'
try 1 'return 0<1;'
try 0 'return 1<1;'
try 0 'return 2<1;'
try 1 'return 0<=1;'
try 1 'return 1<=1;'
try 0 'return 2<=1;'
try 1 'return 1>0;'
try 0 'return 1>1;'
try 0 'return 1>2;'
try 1 'return 1>=0;'
try 1 'return 1>=1;'
try 0 'return 1>=2;'
echo "### step9: 複数の文"
try 1 'return 1; 2; 3;'
try 2 '1; return 2; 3;'
try 3 '1; 2; return 3;'
echo "### step9: 1文字のローカル変数"
try 3 'a=3; return a;'
try 8 'a=3; z=5; return a+z;'
echo "### step10: 複数文字のローカル変数"
try 3 'foo=3; return foo;'
try 8 'foo123=3; bar=5; return foo123+bar;'
echo "### step12: if文"
try 3 'if (0) return 2; return 3;'
try 3 'if (1-1) return 2; return 3;'
try 2 'if (1) return 2; return 3;'
try 2 'if (2-1) return 2; return 3;'

echo ok