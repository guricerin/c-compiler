#!/bin/bash

readonly base_path=$(cd $(dirname $0); pwd)

cat <<EOF | gcc -xc -c -o ./obj/tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
EOF

assert () {
    local -r expected="$1"
    local -r input="$2"
    local -r assembly="${base_path}/obj/tmp.s"
    local -r obj2="${base_path}/obj/tmp2.o"
    local -r binary="${base_path}/obj/tmp"

    ${base_path}/bin/9cc "$input" > "$assembly"
    gcc -static -o "$binary" "$assembly" "$obj2"
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
assert 0 'return 0;'
assert 42 'return 42;'
echo "### step2: 加減算のできるコンパイラの作成"
assert 21 'return 5+20-4;'
echo "### step3: トークナイザを導入"
assert 41 'return  12 + 34 - 5 ;'
echo "### step5: 四則演算のできる言語の作成"
assert 47 'return 5+6*7;'
assert 15 'return 5*(9-6);'
assert 4 'return (3+5)/2;'
echo "### step6: 単項プラスと単項マイナス"
assert 10 'return -10+20;'
assert 10 'return - -10;'
assert 10 'return - - +10;'
echo "### step7: 比較演算子"
assert 0 'return 0==1;'
assert 1 'return 42==42;'
assert 1 'return 0!=1;'
assert 0 'return 42!=42;'
assert 1 'return 0<1;'
assert 0 'return 1<1;'
assert 0 'return 2<1;'
assert 1 'return 0<=1;'
assert 1 'return 1<=1;'
assert 0 'return 2<=1;'
assert 1 'return 1>0;'
assert 0 'return 1>1;'
assert 0 'return 1>2;'
assert 1 'return 1>=0;'
assert 1 'return 1>=1;'
assert 0 'return 1>=2;'
echo "### step9: 複数の文"
assert 1 'return 1; 2; 3;'
assert 2 '1; return 2; 3;'
assert 3 '1; 2; return 3;'
echo "### step9: 1文字のローカル変数"
assert 3 'a=3; return a;'
assert 8 'a=3; z=5; return a+z;'
echo "### step10: 複数文字のローカル変数"
assert 3 'foo=3; return foo;'
assert 8 'foo123=3; bar=5; return foo123+bar;'
echo "### step12: if文"
assert 3 'if (0) return 2; return 3;'
assert 3 'if (1-1) return 2; return 3;'
assert 2 'if (1) return 2; return 3;'
assert 2 'if (2-1) return 2; return 3;'
echo "### step12: while文"
assert 3 'if (0) return 2; return 3;'
assert 10 'i=0; while(i<10) i=i+1; return i;'
echo "### step12: for文"
assert 55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
assert 3 'for (;;) return 3; return 5;'
echo "### step13: ブロック"
assert 3 '{1; {2;} return 3;}'
assert 55 'i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j;'
echo "### step14: 引数のない関数呼び出し"
assert 3 'return ret3();'
assert 5 'return ret5();'

echo ok