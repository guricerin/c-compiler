#include "9cc.h"

char *g_user_input;
Token *g_token;

// エラーを報告し、プログラムを終了する
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告し、プログラムを終了する
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - g_user_input;
    fprintf(stderr, "%s\n", g_user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// トークン先読み
// 次のトークンが期待している記号のときには、トークンを1つ読み進めてtrueを返す
bool consume(char *op)
{
    // 条件の順序に注意
    // 長いトークンから先にトークナイズする必要あり
    if (g_token->kind != TK_RESERVED || strlen(op) != g_token->len || strncmp(g_token->str, op, g_token->len))
        return false;
    g_token = g_token->next;
    return true;
}

Token *consume_ident()
{
    if (g_token->kind != TK_IDENT)
    {
        return NULL;
    }
    else
    {
        Token *t = g_token;
        g_token = g_token->next;
        return t;
    }
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op)
{
    if (g_token->kind != TK_RESERVED || strlen(op) != g_token->len || strncmp(g_token->str, op, g_token->len))
        error_at(g_token->str, "'%s'ではありません", op);
    g_token = g_token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number()
{
    if (g_token->kind != TK_NUM)
        error_at(g_token->str, "数ではありません");
    long val = g_token->val;
    g_token = g_token->next;
    return val;
}

// 次のトークンが識別子の場合、トークンを1つ読み進めてその識別子を返す。
// それ以外の場合にはエラーを報告する。
char *expect_ident()
{
    if (g_token->kind != TK_IDENT)
        error_at(g_token->str, "expected an identifier");
    char *s = strndup(g_token->str, g_token->len);
    g_token = g_token->next;
    return s;
}

bool at_eof()
{
    return g_token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
static Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    // mallocとは違い、callocは割り当てられたメモリを0クリアする
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

static bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

static bool is_alpha(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c)
{
    return is_alpha(c) || ('0' <= c && c <= '9');
}

// 予約語が使用されているならそれを返す
static char *starts_with_reserved(char *p)
{
    // 予約語
    static char *kw[] = {"return", "if", "else", "while", "for"};

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    {
        int len = strlen(kw[i]);
        if (startswith(p, kw[i]) && !is_alnum(p[len]))
        {
            return kw[i];
        }
    }

    // 複数文字の区切り文字
    static char *ops[] = {"==", "!=", "<=", ">="};
    for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++)
    {
        if (startswith(p, ops[i]))
        {
            return ops[i];
        }
    }

    return NULL;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p)
{
    // 連結リストを構築
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // 予約語 or 複数文字の区切り文字
        char *kw = starts_with_reserved(p);
        if (kw)
        {
            int len = strlen(kw);
            cur = new_token(TK_RESERVED, cur, p, len);
            p += len;
            continue;
        }

        // 識別子
        if (is_alpha(*p))
        {
            // 識別子の文字数を算出
            char *q = p++;
            while (is_alnum(*p))
                p++;
            cur = new_token(TK_IDENT, cur, q, p - q);
            continue;
        }

        // 1文字の区切り文字(スペースやタブ文字はfalse)
        if (ispunct(*p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // 整数値リテラル
        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10); // 10進数として解釈
            cur->len = p - q;
            continue;
        }

        error_at(cur->str, "無効なトークンです");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
