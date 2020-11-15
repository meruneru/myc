#ifndef INCLUDED_MYC
#define INCLUDED_MYC

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
extern Token *token;
extern char *user_input;

//parse.c
void error(char *loc, char *fmt, ...);
Token *tokenize(char *p);

bool consume(char* op);
void expect(char* op);
int expect_number();
bool at_eof();

//codegen.c
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

Node* expr();
void gen(Node* node);
#endif
