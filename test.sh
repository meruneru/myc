#!/bin/bash

assert(){
    expected="$1"
    input="$2"

    ./myc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# assert <expected> <input>
assert 0 '0;'
assert 123 '123;'
assert 6 '1+2+3;'
assert 2 '1-2+3;'
assert 24 '(4*2)*3;'
assert 20 '(30*2)/3;'
assert 5 '-3+(4*2);'
assert 11 '+3+(4*2);'
assert 1 '2==2;'
assert 0 '2!=2;'
assert 0 '2<2;'
assert 0 '3<2;'
assert 1 '2<=20;'
assert 0 '2>2;'
assert 1 '3>2;'
assert 7 '1-2+3;7;'

rm tmp tmp.s
echo OK
