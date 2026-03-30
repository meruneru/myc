#include "./myc.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    // tokenizeされたものはcode[]に格納される
    user_input = argv[1];
    token = tokenize(argv[1]);
    Function* prog = program();
    codegen(prog);

    return 0;
}
