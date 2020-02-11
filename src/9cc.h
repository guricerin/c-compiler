#define _GNU_SOURCE

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    type.c
*/

typedef enum
{
    TY_INT,
    TY_PTR,
} TypeKind;

typedef struct Type Type;

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

// 再帰的な構造体を作りたい場合、単純に typedef struct {...} NAME; とするのでは不可能
// トークン
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
void error_tok(Token *tok, char *fmt, ...);
Token *consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
Token *tokenize();

extern char *g_filename;
// 入力プログラム
extern char *g_user_input;
// 現在着目しているトークン
// 入力トークン列を標準入力のようなストリーム(グローバル変数)として扱うほうがパーサのコードが読みやすくなることが多い
extern Token *g_token;

/*
    parse.c
*/

// ローカル変数
typedef struct Var Var;
struct Var
{
    char *name; // 変数名
    int offset; // RBPからのオフセット
};

typedef struct VarList VarList;
struct VarList
{
    VarList *next;
    Var *var;
};

// 抽象構文機のノードの種類
typedef enum
{
    ND_ADD,       // 数値の加算
    ND_PTR_ADD,   // ポインター + 数値
    ND_SUB,       // 数値の減算
    ND_PTR_SUB,   // ポインター - 数値
    ND_PTR_DIFF,  // ポインター - ポインター
    ND_MUL,       // *
    ND_DIV,       // /
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // =
    ND_ADDR,      // 単項&
    ND_DEREF,     // 単項*
    ND_RETURN,    // "return"キーワード
    ND_IF,        // "if"キーワード
    ND_WHILE,     // "while"キーワード
    ND_FOR,       // "for"キーワード
    ND_BLOCK,     // { ... }
    ND_FUNCALL,   // 関数呼び出し
    ND_EXPR_STMT, // 式
    ND_VAR,       // 変数
    ND_NUM,       // 整数
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node
{
    NodeKind kind; // ノードの型
    Token *tok;    // トークン
    Type *ty;      // 値の型
    Node *next;    // 次のノード
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺

    // if or while or for
    Node *cond;
    Node *then;
    Node *els;
    Node *init; // forの初期化文
    Node *inc;  // forの繰り返し文

    Node *body; // ブロック。kindがND_BLOCKの場合のみ使用

    char *funcname; // 関数名。kindがND_FUNCALLの場合のみ使用
    Node *args;     // 引数。kindがND_FUNCALLの場合のみ使用

    Var *var; // ローカル変数。kindがND_VALの場合のみ使用
    long val; // kindがND_NUMの場合のみ使用
};

// 関数
typedef struct Function Function;
struct Function
{
    Function *next;
    char *name;      // 関数名
    VarList *params; // 引数
    Node *node;
    VarList *locals; // ローカル変数
    int stack_size;
};

Function *program();

/*
    type.c
*/

struct Type
{
    TypeKind kind;
    Type *base; // 参照先の型。kindがTY_PTRの場合に使用
};

bool is_integer(Type *ty);
void add_type(Node *node);

/*
    codegen.c
*/

void codegen(Function *prog);