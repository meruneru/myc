#include <stdio.h>

int foo() {
    printf("OK\n");
    return 42;
}

int add(int x, int y) {
    return x + y;
}

int add6(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}

void alloc4(int **p, int a, int b, int c, int d) {
    static int arr[4];
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d;
    *p = arr;
}
