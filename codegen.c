#include "./myc.h"

Node* code[100];

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
    Node* new = calloc(1, sizeof(Node));
    new->kind = kind;
    new->lhs = lhs;
    new->rhs = rhs;
    return new;
}

Node* new_node_num(int val) {
    Node* new = calloc(1, sizeof(Node));
    new->kind = ND_NUM;
    new->val = val;
    return new;
}

Node* stmt();
Node* expr();
Node* equality();
Node* assign();
Node* relational();
Node* add();
Node* mul();
Node* primary();
Node* unary();

// program    = stmt*
void program() {
    int i = 0;

    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

// stmt       = expr ";"
Node* stmt() {
    Node* node = expr();
    expect(";");
    return node;
}

// expr       = assign
Node* expr() { return assign(); }

// assign     = equality ("=" assign)?
Node* assign() {
    Node* node = equality();

    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node* equality() {
    Node* node = relational();
    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational() {
    Node* node = add();
    for (;;) {
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

// add    = mul ("+" mul | "-" mul)*
Node* add() {
    Node* node = mul();

    for (;;) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// mul     = unary ("*" unary | "/" unary)*
Node* mul() {
    Node* node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}
// unary   = ("+" | "-")? primary
Node* unary() {
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}
// primary = num | "(" expr ")"
// primary    = num | ident | "(" expr ")"
Node* primary() {
    Token* tok = consume_ident();
    if (tok) {
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    }
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}

void gen_lval(Node* node) {
    if (node->kind != ND_LVAR) {
        printf("代入の左辺値は変数ではありません");
        exit(1);
    }

    printf("mov rax, rbp\n");
    printf("sub rax, %d\n", node->offset);
    printf("push rax\n");
}

void gen(Node* node) {
    switch (node->kind) {
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
    }
    printf("  push rax\n");
}

