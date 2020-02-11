#include "9cc.h"

// すべてのローカル変数をこの連結リストに格納する
static VarList *g_locals;

// ローカル変数を変数名で検索
static Var *find_var(Token *tok)
{
    for (VarList *vl = g_locals; vl; vl = vl->next)
    {
        Var *var = vl->var;
        if (strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len))
            return var;
    }
    return NULL;
}

static Node *new_node(NodeKind kind, Token *tok)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->tok = tok;
    return node;
}

// 二項演算
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
    Node *node = new_node(kind, tok);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 単項演算子
static Node *new_unary(NodeKind kind, Node *expr, Token *tok)
{
    Node *node = new_node(kind, tok);
    node->lhs = expr;
    return node;
}

static Node *new_node_num(long val, Token *tok)
{
    Node *node = new_node(ND_NUM, tok);
    node->val = val;
    return node;
}

static Node *new_var_node(Var *var, Token *tok)
{
    Node *node = new_node(ND_VAR, tok);
    node->var = var;
    return node;
}

// ローカル変数を生成し、ローカル変数リストに追加
static Var *new_lvar(char *name)
{
    Var *var = calloc(1, sizeof(Var));
    var->name = name;

    VarList *vl = calloc(1, sizeof(VarList));
    vl->var = var;
    vl->next = g_locals;
    g_locals = vl;
    return var;
}

static Function *function();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

// program = function*
Function *program()
{
    Function head = {};
    Function *cur = &head;

    while (!at_eof())
    {
        cur->next = function();
        cur = cur->next;
    }
    return head.next;
}

static Node *read_expr_stmt()
{
    Token *tok = g_token;
    return new_unary(ND_EXPR_STMT, expr(), tok);
}

static VarList *read_func_params()
{
    if (consume(")"))
        return NULL;

    VarList *head = calloc(1, sizeof(VarList));
    head->var = new_lvar(expect_ident());
    VarList *cur = head;

    while (!consume(")"))
    {
        expect(",");
        cur->next = calloc(1, sizeof(VarList));
        cur->next->var = new_lvar(expect_ident());
        cur = cur->next;
    }

    return head;
}

// function = ident "(" ")" "{" stmt* "}"
static Function *function()
{
    g_locals = NULL;

    Function *fn = calloc(1, sizeof(Function));
    fn->name = expect_ident();
    expect("(");
    fn->params = read_func_params();
    expect("{");

    Node head = {};
    Node *cur = &head;

    while (!consume("}"))
    {
        cur->next = stmt();
        cur = cur->next;
    }

    fn->node = head.next;
    fn->locals = g_locals; // 現時点で定義されているローカル変数
    return fn;
}

// stmt = "return" expr ";"
//      | expr ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
static Node *stmt()
{
    Token *tok;
    if (tok = consume("return"))
    {
        Node *node = new_unary(ND_RETURN, expr(), tok);
        expect(";");
        return node;
    }
    else if (tok = consume("if"))
    {
        Node *node = new_node(ND_IF, tok);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else"))
        {
            node->els = stmt();
        }
        return node;
    }
    else if (tok = consume("while"))
    {
        Node *node = new_node(ND_WHILE, tok);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    }
    else if (tok = consume("for"))
    {
        Node *node = new_node(ND_FOR, tok);
        expect("(");
        if (!consume(";"))
        {
            node->init = read_expr_stmt();
            expect(";");
        }
        if (!consume(";"))
        {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")"))
        {
            node->inc = read_expr_stmt();
            expect(")");
        }
        node->then = stmt();
        return node;
    }
    else if (tok = consume("{")) // ブロック
    {
        Node head = {};
        Node *cur = &head;
        while (!consume("}"))
        {
            cur->next = stmt();
            cur = cur->next;
        }
        Node *node = new_node(ND_BLOCK, tok);
        node->body = head.next;
        return node;
    }
    else
    {
        Node *node = read_expr_stmt();
        expect(";");
        return node;
    }
}

// expr = assign
static Node *expr()
{
    return assign();
}

// assign = equality ("=" assign)?
static Node *assign()
{
    Node *node = equality();
    Token *tok;
    if (tok = consume("="))
        node = new_binary(ND_ASSIGN, node, assign(), tok);
    return node;
}

// equality = relational ("==" relational | "!=" relational)
static Node *equality()
{
    Node *node = relational();
    Token *tok;
    for (;;)
    {
        if (tok = consume("=="))
            node = new_binary(ND_EQ, node, relational(), tok);
        else if (tok = consume("!="))
            node = new_binary(ND_NE, node, relational(), tok);
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational()
{
    Node *node = add();
    Token *tok;

    for (;;)
    {
        if (tok = consume("<"))
            node = new_binary(ND_LT, node, add(), tok);
        else if (tok = consume("<="))
            node = new_binary(ND_LE, node, add(), tok);
        // 左辺と右辺を単にひっくり返す
        else if (tok = consume(">"))
            node = new_binary(ND_LT, add(), node, tok);
        else if (tok = consume(">="))
            node = new_binary(ND_LE, add(), node, tok);
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
static Node *add()
{
    Node *node = mul();
    Token *tok;

    for (;;)
    {
        if (tok = consume("+"))
            node = new_binary(ND_ADD, node, mul(), tok);
        else if (tok = consume("-"))
            node = new_binary(ND_SUB, node, mul(), tok);
        else
            return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul()
{
    Node *node = unary();
    Token *tok;

    for (;;)
    {
        if (tok = consume("*"))
            node = new_binary(ND_MUL, node, unary(), tok);
        else if (tok = consume("/"))
            node = new_binary(ND_DIV, node, unary(), tok);
        else
            return node;
    }
}

// unary = ("+" | "-")? unary | primary
static Node *unary()
{
    Token *tok;
    if (tok = consume("+"))
        return unary();
    else if (tok = consume("-"))
        return new_binary(ND_SUB, new_node_num(0, tok), unary(), tok);
    else
        return primary();
}

// func-args = "(" (assign ("," assign)*)? ")"
static Node *func_args()
{
    if (consume(")"))
    {
        return NULL;
    }

    Node *head = assign();
    Node *cur = head;
    while (consume(","))
    {
        cur->next = assign();
        cur = cur->next;
    }
    expect(")");
    return head;
}

// primary = "(" expr ")"
//         | ident func-args?
//         | num
static Node *primary()
{
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok;
    // 識別子なら変数名のはず
    if (tok = consume_ident())
    {
        // 関数呼び出し
        if (consume("("))
        {
            Node *node = new_node(ND_FUNCALL, tok);
            node->funcname = strndup(tok->str, tok->len);
            node->args = func_args();
            return node;
        }

        Var *var = find_var(tok);
        // ローカル変数リストになければ新規生成
        if (!var)
        {
            // strndup: 文字列を複製するGCC拡張関数
            var = new_lvar(strndup(tok->str, tok->len));
        }
        return new_var_node(var, tok);
    }

    // そうでなければ数値のはず
    tok = g_token;
    if (tok->kind != TK_NUM)
        error_tok(tok, "数値ではありません");
    return new_node_num(expect_number(), tok);
}
