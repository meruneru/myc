#!/bin/bash

assert(){
    expected="$1"
    input="$2"

    ./build/myc "$input" > tmp.s
    if [ $? -ne 0 ]; then
        echo "Compile failed: $input"
        exit 1
    fi
    cc -o tmp tmp.s build/test_helper.o
    if [ $? -ne 0 ]; then
        echo "Link failed: $input"
        exit 1
    fi
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
assert 0 'main() { return 0; }'
assert 123 'main() { return 123; }'
assert 6 'main() { return 1+2+3; }'
assert 2 'main() { return 1-2+3; }'
assert 24 'main() { return (4*2)*3; }'
assert 20 'main() { return (30*2)/3; }'
assert 5 'main() { return -3+(4*2); }'
assert 11 'main() { return +3+(4*2); }'
assert 1 'main() { return 2==2; }'
assert 0 'main() { return 2!=2; }'
assert 0 'main() { return 2<2; }'
assert 0 'main() { return 3<2; }'
assert 1 'main() { return 2<=20; }'
assert 0 'main() { return 2>2; }'
assert 1 'main() { return 3>2; }'
assert 7 'main() { 1-2+3; return 7; }'
assert 3 'main() { a = 3; return a; }'
assert 22 'main() { b = 5*6-8; return b; }'
assert 14 'main() { a = 3; b = 5*6-8; b = a+b/2; return b; }'
assert 6 'main() { foo=1; bar=5; return foo+bar; }'
assert 3 'main() { foo=3; bar=5; return foo; }'
assert 10 'main() { foo=0; if(foo==0) foo=10; return foo; }'
assert 10 'main() { foo=0; if(foo==0) { foo=10; } return foo; }'
assert 10 'main() { foo=0; if(foo==0) foo=10; else foo=5; return foo; }'
assert 10 'main() { foo=1; if(foo) foo=10; else foo=5; return foo; }'
assert 5 'main() { foo=1; if(foo==0) foo=10; else foo=5; return foo; }'
assert 5 'main() { foo=0; while(foo<5) foo=foo+1; return foo; }'
assert 9 'main() { foo=0; for(bar=0; bar<10; bar=bar+1) foo=bar; return foo; }'
assert 10 'main() { foo=0; for(bar=0; bar<10; bar=bar+1) foo=bar; return bar; }'

# Function call tests
assert 42 'main() { return foo(); }'
assert 10 'main() { return add(3, 7); }'
assert 21 'main() { return add6(1, 2, 3, 4, 5, 6); }'

# Function definition tests
assert 7 'main() { return add2(3, 4); } add2(x, y) { return x + y; }'
assert 1 'main() { return sub2(4, 3); } sub2(x, y) { return x - y; }'
assert 55 'main() { return fib(10); } fib(n) { if(n<=1) return n; return fib(n-1) + fib(n-2); }'

rm tmp tmp.s
echo OK
