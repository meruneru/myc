#!/bin/bash

assert(){
    expected="$1"
    input="$2"

    ./build/myc "$input" > tmp.s
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
make clean
make
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
assert 3 'a = 3;'
assert 22 'b = 5*6-8;'
assert 14 'a = 3;b = 5*6-8;b = a+b/2;'
assert 6 'foo=1;bar=5;foo+bar;'
assert 3 'foo=3;bar=5;return foo;'
assert 10 'foo=0;if(foo==0)foo=10;'
assert 10 'foo=0;if(foo==0)foo=10;'
assert 10 'foo=0;if(foo==0)foo=10;else foo=5;'
assert 10 'foo=1;if(foo)foo=10;else foo=5;'
assert 5 'foo=1;if(foo==0)foo=10;else foo=5;'
assert 5 'foo=0;while(foo<5)foo=foo+1;'
assert 9 'foo=0;for(bar=0; bar<10; bar=bar+1)foo=bar;return foo;'
assert 10 'foo=0;for(bar=0; bar<10; bar=bar+1)foo=bar;return bar;'
assert 10 'foo=0;bar=0;for(; bar<10; bar=bar+1)foo=bar;return bar;'
assert 10 'foo=0;bar=0;for(; bar<10; )bar=bar+1;return bar;'
assert 20 'foo=0;bar=0;for(; bar<10; ){bar=bar+1;foo=foo+2;}return foo;'
assert 10 'foo=0;if(foo==0){foo=10;}return foo;'

rm tmp tmp.s
echo OK
