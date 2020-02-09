#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    tokenize.c
*/

// トークンの種類
typedef enum
{
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数
    TK_EOF,      // 入力の終わりを表す
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークンの文字列
    int len;        // トークンの長さ
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *tokenize();

extern char *filename;
// 入力プログラム
extern char *user_input;
// 現在着目しているトークン
// 入力トークン列を標準入力のようなストリーム(グローバル変数)として扱うほうがパーサのコードが読みやすくなることが多い
extern Token *token;

/*
    parse.c
*/

// 抽象構文機のノードの種類
typedef enum
{
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // =
    ND_RETURN,    // "return"
    ND_EXPR_STMT, // 式
    ND_VAR,       // 変数
    ND_NUM,       // 整数
} NodeKind;

// 再帰的な構造体を作りたい場合、単純に typedef struct {...} NAME; とするのでは不可能
typedef struct Node Node;
// 抽象構文機のノードの型
struct Node
{
    NodeKind kind; // ノードの型
    Node *next;    // 次のノード
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    char name;     // 変数名。kindがND_VALの場合のm使用
    long val;      // kindがND_NUMの場合のみ使用
};

Node *program();

/*
    codegen.c
*/
void codegen(Node *node);