#include "9cc.h"

// すべてのローカル変数をこの連結リストに格納する
Var *locals;

// ローカル変数を変数名で検索
static Var *find_var(Token *tok)
{
    for (Var *var = locals; var; var = var->next)
    {
        if (strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len))
            return var;
    }
    return NULL;
}

static Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

// 二項演算
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 単項演算子
static Node *new_unary(NodeKind kind, Node *expr)
{
    Node *node = new_node(kind);
    node->lhs = expr;
    return node;
}

static Node *new_node_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

static Node *new_var_node(Var *var)
{
    Node *node = new_node(ND_VAR);
    node->var = var;
    return node;
}

// ローカル変数を生成し、ローカル変数リストに追加
static Var *new_lvar(char *name)
{
    Var *var = calloc(1, sizeof(Var));
    var->next = locals;
    var->name = name;
    locals = var;
    return var;
}

static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

// program = stmt*
Function *program()
{
    locals = NULL;
    Node head = {};
    Node *cur = &head;

    while (!at_eof())
    {
        cur->next = stmt();
        cur = cur->next;
    }

    Function *prog = calloc(1, sizeof(Function));
    prog->node = head.next;
    prog->locals = locals;
    return prog;
}

static Node *read_expr_stmt()
{
    return new_unary(ND_EXPR_STMT, expr());
}

// stmt = "return" expr ";"
//      | expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
Node *stmt()
{
    if (consume("return"))
    {
        Node *node = new_unary(ND_RETURN, expr());
        expect(";");
        return node;
    }
    else if (consume("if"))
    {
        Node *node = new_node(ND_IF);
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
    else
    {
        Node *node = new_unary(ND_EXPR_STMT, expr());
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
    if (consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    return node;
}

// equality = relational ("==" relational | "!=" relational)
Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
            node = new_binary(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_binary(ND_NE, node, relational());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume("<"))
            node = new_binary(ND_LT, node, add());
        else if (consume("<="))
            node = new_binary(ND_LE, node, add());
        // 左辺と右辺を単にひっくり返す
        else if (consume(">"))
            node = new_binary(ND_LT, add(), node);
        else if (consume(">="))
            node = new_binary(ND_LE, add(), node);
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ("+" | "-")? unary | primary
Node *unary()
{
    if (consume("+"))
        return unary();
    else if (consume("-"))
        return new_binary(ND_SUB, new_node_num(0), unary());
    else
        return primary();
}

// primary = "(" expr ")" | ident | num
Node *primary()
{
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    // 識別子なら変数名のはず
    Token *tok = consume_ident();
    if (tok)
    {
        Var *var = find_var(tok);
        // ローカル変数リストになければ新規生成
        if (!var)
        {
            // strndup: 文字列を複製するGCC拡張関数
            var = new_lvar(strndup(tok->str, tok->len));
        }
        return new_var_node(var);
    }

    // そうでなければ数値のはず
    return new_node_num(expect_number());
}
