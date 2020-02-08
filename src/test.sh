#!/bin/bash

try () {
    local -r expected="$1"
    local -r input="$2"

    gcc -o 9cc 9cc.c
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
echo ok