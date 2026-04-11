#include "./myc.h"
int labelCnt;
char* argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node* node);
void gen(Node* node);

void codegen(Function* prog) {
    printf(".intel_syntax noprefix\n");
    for (Function* fn = prog; fn; fn = fn->next) {
        printf(".globl %s\n", fn->name);
        printf("%s:\n", fn->name);

        // プロローグ
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        // スタックサイズを16バイト境界に切り上げる
        int stack_size = (fn->stack_size + 15) / 16 * 16;
        printf("  sub rsp, %d\n", stack_size);

        // 引数をスタックにコピー
        int i = 0;
        for (Node* param = fn->params; param; param = param->next) {
            printf("  mov [rbp-%d], %s\n", param->offset, argreg[i++]);
        }

        gen(fn->body);
        printf("  pop rax\n");

        // エピローグ
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}

// 右辺値として評価してスタックにプッシュ
void gen(Node* node) {
    if (node == NULL) return;
    switch (node->kind) {
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_FUNC: {
            int nargs = 0;
            for (Node* arg = node->args; arg; arg = arg->next) {
                gen(arg);
                nargs++;
            }

            for (int i = nargs - 1; i >= 0; i--) {
                printf("  pop %s\n", argreg[i]);
            }

            int labelNum = labelCnt++;
            printf("  mov rax, rsp\n");
            printf("  and rax, 15\n");
            printf("  jnz .L.call.%d\n", labelNum);

            printf("  mov rax, 0\n");
            printf("  call %.*s\n", node->len, node->funcname);
            printf("  jmp .L.end.%d\n", labelNum);

            printf(".L.call.%d:\n", labelNum);
            printf("  sub rsp, 8\n");
            printf("  mov rax, 0\n");
            printf("  call %.*s\n", node->len, node->funcname);
            printf("  add rsp, 8\n");

            printf(".L.end.%d:\n", labelNum);
            printf("  push rax\n");
            return;
        }
        case ND_BLOCK: {
            for (Node* n = node->body; n != NULL; n = n->next) {
                gen(n);
                printf("  pop rax\n");
            }
            printf("  push 0\n");
            return;
        }
        case ND_IF: {
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            int labelNum = labelCnt++;
            if (node->els == NULL) {
                printf("  je .Lend%d\n", labelNum);
                gen(node->then);
            } else {
                printf("  je .Lelse%d\n", labelNum);
                gen(node->then);
                printf("  jmp .Lend%d\n", labelNum);
                printf(".Lelse%d:\n", labelNum);
                gen(node->els);
            }
            printf(".Lend%d:\n", labelNum);
            return;
        }
        case ND_WHILE: {
            int labelNum = labelCnt++;
            printf(".Lbegin%d:\n", labelNum);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", labelNum);
            gen(node->then);
            printf("  jmp .Lbegin%d\n", labelNum);
            printf(".Lend%d:\n", labelNum);
            return;
        }
        case ND_FOR: {
            int labelNum = labelCnt++;
            gen(node->init);
            printf(".Lbegin%d:\n", labelNum);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", labelNum);
            gen(node->then);
            gen(node->step);
            printf("  jmp .Lbegin%d\n", labelNum);
            printf(".Lend%d:\n", labelNum);
            return;
        }
        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
        default:
            // do noting
            break;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        default:
            // do noting
            break;
    }
    printf("  push rax\n");
}

// 左辺値として評価してスタックにプッシュ
void gen_lval(Node* node) {
    if (node->kind == ND_DEREF) {
        gen(node->lhs);
        return;
    }

    if (node->kind != ND_LVAR) {
        printf("代入の左辺値は変数ではありません");
        exit(1);
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}
