#include "./myc.h"

// 現在着目しているトークン
Token* token;
char* user_input;
LVar* locals;

void error(char* loc, char* fmt, ...) {
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

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar* find_lvar(Token* tok) {
    for (LVar* var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

// 次のトークンが期待している記号のときは、トークンを1つ進めて真を返す。
// それ以外の場合、現在のトークンを維持したまま偽を返す。
bool consume(char* op) {
    if ((token->kind != TK_RESERVED && token->kind != TK_RETURN) ||
        token->len != strlen(op) || memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが識別子のとき、トークンを1つ進めてそのトークンを返す。
// それ以外の場合、現在のトークンを維持したままNULLを返す。
Token* consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token* t = token;
    token = token->next;
    return t;
}

// 次のトークンが期待している記号のとき、トークンを1つ進める。
// それ以外の記号が来た場合は、致命的なエラーを報告する。
void expect(char* op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) ||
        memcmp(token->str, op, token->len)) {
        error(token->str, "%sではありません", op);
    }
    token = token->next;
}

// 次のトークンが数値であるとき、トークンを1つ進めてその数値を返す。
// 数値以外が来た場合は、致命的なエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM) {
        error(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// 新しいトークンを作成してcurにつなげる
Token* new_token(TokenKind kind, Token* cur, char* str) {
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

bool startswith(char* str1, char* str2) {
    return strncmp(str1, str2, strlen(str2)) == 0;
}

int is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}

int is_alpha1(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

int is_alpha2(char c) { return is_alpha1(c) || ('0' <= c && c <= '9'); }

Token* tokenize(char* p) {
    Token head;
    head.next = NULL;
    Token* cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (strncmp(p, "return", 6) == 0 && !is_alpha(p[6])) {
            cur = new_token(TK_RETURN, cur, p);
            int len = strlen("return");
            cur->len = len;
            p += len;
            continue;
        }
        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
            startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 2;
            p += 2;
            continue;
        }
        if (strchr("+-*/()<>;=", *p)) {
            cur = new_token(TK_RESERVED, cur, p++);
            cur->len = 1;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            char* q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        if (is_alpha1(*p)) {
            char* start = p;
            do {
                p++;
            } while (is_alpha2(*p));
            cur = new_token(TK_IDENT, cur, start);
            cur->len = p - start;
            continue;
        }
        error(p, "トークナイズできません");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}
