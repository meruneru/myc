#ifndef INCLUDED_MYC
#define INCLUDED_MYC

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,  // 記号
    TK_IDENT,     // 識別子
    TK_NUM,       // 整数トークン
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_FOR,
    TK_WHILE,
    TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token* next;
    int val;
    char* str;
    int len;
};

// ローカル変数の型
typedef struct LVar LVar;
struct LVar {
    LVar* next;  // 次の変数かNULL
    char* name;  // 変数の名前
    int len;     // 名前の長さ
    int offset;  // RBPからのオフセット
};

// 現在着目しているトークン
extern Token* token;
extern char* user_input;
extern LVar* locals;

// parse.c
void error(char* loc, char* fmt, ...);
LVar* find_lvar(Token* tok);
Token* tokenize(char* p);

bool consume(char* op);
Token* consume_ident();
void expect(char* op);
int expect_number();
bool at_eof();

typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_ASSIGN,  // =
    ND_LVAR,    // ローカル変数
    ND_EQ,      // ==
    ND_NE,      // !=
    ND_LT,      // <
    ND_LE,      // <=
    ND_NUM,
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_FUNC,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node* rhs;
    Node* lhs;

    // if, while
    Node* cond;  // 条件式
    Node* then;  // 真
    Node* els;   // 偽
    // for only
    Node* init;  // 初期化
    Node* step;  // 更新条件

    // block
    Node* body;
    Node* next;

    // function call
    char* funcname;
    Node* args;
    int len;

    int val;     // kind==ND_NUM only
    int offset;  // kind==ND_LVAR only
};

extern Node* code[100];

Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
void program();
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();

// codegen.c
void gen(Node* node);
#endif
