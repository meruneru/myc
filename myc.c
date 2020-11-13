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
    int len;
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
bool consume(char* op) {
    if (token->kind != TK_RESERVED ||
            token->len != strlen(op) ||
            memcmp(token->str, op, token->len) ) {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときは、トークンを１つ進める
void expect(char* op) {
    if (token->kind != TK_RESERVED ||
            token->len != strlen(op) ||
            memcmp(token->str, op, token->len) ){
        error(token->str, "%sではありません", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char* str1, char* str2){
    return strncmp(str1, str2, strlen(str2)) == 0;
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
        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p+=2;
            continue;
        }
        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char* q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        error(cur->str, "トークナイズできません");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

typedef enum{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
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
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* primary();
Node* unary();

// expr       = equality
Node* expr(){
    return equality();
}

// equality   = relational ("==" relational | "!=" relational)*
Node* equality(){
    Node* node = relational();   
    for(;;){
        if(consume("==")){
            node=new_node(ND_EQ, node, relational());
        }else if(consume("!=")){
            node=new_node(ND_NE, node, relational());
        }else{
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational(){
    Node* node = add();
    for(;;){
        if(consume("<")){
            node=new_node(ND_LT, node, add());
        }else if(consume("<=")){
            node=new_node(ND_LE, node, add());
        }else if(consume(">")){
            node=new_node(ND_LT, add(), node);
        }else if(consume(">=")){
            node=new_node(ND_LE, add(), node);
        }else{
            return node;
        }
    }

}

// add    = mul ("+" mul | "-" mul)*
Node* add(){
    Node* node=mul();

    for(;;){
        if(consume("+")){
            node=new_node(ND_ADD, node, mul());
        }else if(consume("-")){
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
        if(consume("*")){
            node=new_node(ND_MUL, node, unary());
        }else if(consume("/")){
            node=new_node(ND_DIV, node,unary());
        }else{
            return node;
        }
    }
}
// unary   = ("+" | "-")? primary
Node* unary(){
    if(consume("+")){
        return primary();
    }else if(consume("-")){
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}
// primary = num | "(" expr ")"
Node* primary(){
    if(consume("(")){
        Node* node=expr();
        expect(")");
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

