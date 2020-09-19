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
assert 0 0
assert 123 123

rm tmp tmp.s
echo OK
