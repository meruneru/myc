#ifndef INCLUDED_MYC
#define INCLUDED_MYC

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,  //記号
    TK_IDENT,     //識別子
    TK_NUM,       //整数トークン
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
extern Token *token;
extern char *user_input;

// parse.c
void error(char *loc, char *fmt, ...);
Token *tokenize(char *p);

bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();

// codegen.c
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
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *rhs;
    Node *lhs;
    int val;     // kind==ND_NUM only
    int offset;  // kind==ND_LVAR only
};

extern Node *code[100];

void program();
void gen(Node *node);
#endif
