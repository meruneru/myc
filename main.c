#include "./myc.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    // tokenizeされたものはcode[]に格納される
    user_input = argv[1];
    token = tokenize(argv[1]);
    program();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個の領域確保
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        //式の評価結果としてスタックに一つの値が残っている
        //スタックが溢れないようポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXにあるのでそれを返す

    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
