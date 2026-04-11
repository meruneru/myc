#include "./myc.h"

// 現在着目しているトークン
Token* token;
char* user_input;
LVar* locals;

Node* code[100];

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
    if ((token->kind != TK_RESERVED && token->kind != TK_RETURN &&
         token->kind != TK_IF && token->kind != TK_ELSE &&
         token->kind != TK_WHILE && token->kind != TK_FOR &&
         token->kind != TK_INT) ||
        token->len != (int)strlen(op) || memcmp(token->str, op, token->len)) {
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
    if ((token->kind != TK_RESERVED && token->kind != TK_RETURN &&
         token->kind != TK_IF && token->kind != TK_ELSE &&
         token->kind != TK_WHILE && token->kind != TK_FOR &&
         token->kind != TK_INT) ||
        token->len != (int)strlen(op) || memcmp(token->str, op, token->len)) {
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
        if (strchr("+-*/()<>;={},&", *p)) {
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
        if (strncmp(p, "int", 3) == 0 && !is_alpha(p[3])) {
            cur = new_token(TK_INT, cur, p);
            cur->len = 3;
            p += 3;
            continue;
        }
        if (startswith(p, "if")) {
            cur = new_token(TK_IF, cur, p);
            cur->len = 2;
            p += 2;
            continue;
        }
        if (startswith(p, "else")) {
            cur = new_token(TK_ELSE, cur, p);
            cur->len = 4;
            p += 4;
            continue;
        }
        if (startswith(p, "for")) {
            cur = new_token(TK_FOR, cur, p);
            cur->len = 3;
            p += 3;
            continue;
        }
        if (startswith(p, "while")) {
            cur = new_token(TK_WHILE, cur, p);
            cur->len = 5;
            p += 5;
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

Function* function();

// program    = function*
Function* program() {
    Function head = {};
    Function* cur = &head;

    while (!at_eof()) {
        cur->next = function();
        cur = cur->next;
    }
    return head.next;
}

// function   = "int" ident "(" ("int" "*"* ident ("," "int" ident)*)? ")" stmt
Function* function() {
    locals = NULL;
    Function* fn = calloc(1, sizeof(Function));

    expect("int");
    Token* tok = consume_ident();
    if (!tok) error(token->str, "関数名がありません");
    fn->name = calloc(tok->len + 1, sizeof(char));
    memcpy(fn->name, tok->str, tok->len);

    expect("(");
    if (!consume(")")) {
        Node head = {};
        Node* cur = &head;
        do {
            expect("int");
            Type ty = {INT, NULL};
            while (consume("*")) {
                Type next = {PTR, calloc(1, sizeof(Type))};
                *next.ptr_to = ty;
                ty = next;
            }
            Token* t = consume_ident();
            if (!t) error(token->str, "引数名がありません");
            Node* node = calloc(1, sizeof(Node));
            node->kind = ND_LVAR;
            LVar* lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = t->str;
            lvar->len = t->len;
            lvar->offset = (locals ? locals->offset : 0) + 8;
            lvar->type = ty;
            node->offset = lvar->offset;
            locals = lvar;

            cur->next = node;
            cur = cur->next;
        } while (consume(","));
        fn->params = head.next;
        expect(")");
    }

    fn->body = stmt();
    fn->locals = locals;
    fn->stack_size = (locals ? locals->offset : 0);
    return fn;
}

// stmt       = expr ";"
//            | "int" "*"* ident ";"
//            | "{" stmt* "}"
//            | "if" "(" expr ")" stmt ("else" stmt)?
//            | "while" "(" expr ")" stmt
//            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//            | return expr ";"
Node* stmt() {
    Node* node;

    if (consume("int")) {
        Type ty = {INT, NULL};
        while (consume("*")) {
            Type next = {PTR, calloc(1, sizeof(Type))};
            *next.ptr_to = ty;
            ty = next;
        }
        Token* tok = consume_ident();
        if (!tok) error(token->str, "識別子ではありません");

        LVar* lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = (locals ? locals->offset : 0) + 8;
        lvar->type = ty;
        locals = lvar;

        expect(";");
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        return node;  // ND_BLOCK (body=NULL) as dummy
    }

    if (consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        Node head = {};
        Node* cur = &head;
        while (!consume("}")) {
            cur->next = stmt();
            cur = cur->next;
        }
        node->body = head.next;
        return node;
    } else if (consume("if")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else")) {
            node->els = stmt();
        }
        return node;
    } else if (consume("while")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    } else if (consume("for")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        //"for" "(" expr? ";" expr? ";" expr? ")" stmt
        //           ^
        if (consume(";")) {
            // no expr
        } else {
            node->init = expr();
            expect(";");
        }
        //"for" "(" expr? ";" expr? ";" expr? ")" stmt
        //                     ^
        if (consume(";")) {
            // no expr
        } else {
            node->cond = expr();
            expect(";");
        }
        //"for" "(" expr? ";" expr? ";" expr? ")" stmt
        //                                ^
        if (consume(")")) {
            // no expr
        } else {
            node->step = expr();
            expect(")");
        }
        node->then = stmt();
        return node;
    } else if (consume("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else {
        node = expr();
    }
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
//          | "*" unary
//          | "&" unary
Node* unary() {
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else if (consume("*")) {
        return new_node(ND_DEREF, unary(), NULL);
    } else if (consume("&")) {
        return new_node(ND_ADDR, unary(), NULL);
    }
    return primary();
}
// primary = num
//          | ident ("(" (expr ("," expr)*)? ")")?
//          | "(" expr ")"
Node* primary() {
    Token* tok = consume_ident();
    if (tok) {
        if (consume("(")) {
            Node* node = calloc(1, sizeof(Node));
            node->kind = ND_FUNC;
            node->funcname = tok->str;
            node->len = tok->len;

            if (consume(")")) {
                return node;
            }

            Node head = {};
            Node* cur = &head;
            do {
                cur->next = expr();
                cur = cur->next;
            } while (consume(","));
            expect(")");
            node->args = head.next;
            return node;
        }

        LVar* lvar = find_lvar(tok);
        if (!lvar) {
            error(tok->str, "定義されていない変数です");
        }
        Node* node = calloc(1, sizeof(Node));
        node->offset = lvar->offset;
        node->kind = ND_LVAR;
        return node;
    }
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}
