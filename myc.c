#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    TK_RESERVED, //Sign
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

// 現在着目しているトークン
Token *token;
char *user_input;

void error(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときは、トークンを１つ進める
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときは、トークンを１つ進める
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error(token->str, "%cではありません", op);
    }
    token = token->next;
}

// 次のトークンが数値のときは、トークンを１つ進めて、数値を返す
int expect_number() {
    if (token->kind != TK_NUM) {
        error(token->str, "数ではありません");
    }
    int val = token->val;
    token = token -> next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (strchr("+-*/()", *p)) {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        error(cur->str, "トークナイズできません");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}

typedef enum{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;
struct Node{
    NodeKind kind;
    Node* rhs;
    Node* lhs;
    int val; // kind==ND_NUM only
};

Node* new_node(NodeKind kind, Node* lhs, Node* rhs){
    Node* new = calloc(1, sizeof(Node));
    new->kind = kind;
    new->lhs = lhs;
    new->rhs = rhs;
    return new;
}

Node* new_node_num(int val){
    Node* new = calloc(1, sizeof(Node));
    new->kind = ND_NUM;
    new->val = val;
    return new;
}

Node* expr();
Node* mul();
Node* primary();
Node* unary();
// expr    = mul ("+" mul | "-" mul)*
Node* expr(){
    Node* node=mul();

    for(;;){
        if(consume('+')){
            node=new_node(ND_ADD, node, mul());
        }else if(consume('-')){
            node=new_node(ND_SUB, node, mul());
        }else{
            return node;
        }
    }
}

// mul     = unary ("*" unary | "/" unary)*
Node* mul(){
    Node* node=unary();

    for(;;){
        if(consume('*')){
            node=new_node(ND_MUL, node, unary());
        }else if(consume('/')){
            node=new_node(ND_DIV, node,unary());
        }else{
            return node;
        }
    }
}
// unary   = ("+" | "-")? primary
Node* unary(){
    if(consume('+')){
        return primary();
    }else if(consume('-')){
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}
// primary = num | "(" expr ")"
Node* primary(){
    if(consume('(')){
        Node* node=expr();
        expect(')');
        return node;
    }
    return new_node_num(expect_number());
}

void gen(Node* node){
    if(node->kind == ND_NUM){
        printf("  push %d\n", node->val);
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->kind){
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
    }
    printf("  push rax\n");
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    user_input = argv[1];
    token = tokenize(argv[1]);
    Node* node = expr();
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}

