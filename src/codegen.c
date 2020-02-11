#include "9cc.h"

// アセンブリにおけるラベルの通し番号
static int g_labelseq = 1;
static char *g_argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static char *g_funcname;

static void gen_addr(Node *node)
{
    if (node->kind == ND_VAR)
    {
        // スタックポインタからのオフセットで変数のアドレスを計算し、スタックにプッシュ
        printf("    lea rax, [rbp-%d]\n", node->var->offset);
        printf("    push rax\n");
        return;
    }

    error("代入文の左辺値が変数ではありません");
}

// メモリから値を読み込む
static void load()
{
    printf("    pop rax\n");
    printf("    mov rax, [rax]\n"); // RAXに入っているアドレスから値を読み込み、RAXにセット
    printf("    push rax\n");
}

// メモリに値を格納
static void store()
{
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    mov [rax], rdi\n");
    printf("    push rdi\n");
}

// 抽象構文木をアセンブリに変換
static void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %ld\n", node->val);
        return;
    case ND_EXPR_STMT:
        gen(node->lhs);
        printf("    add rsp, 8\n");
        return;
    case ND_VAR:
        gen_addr(node);
        load();
        return;
    case ND_ASSIGN:
        gen_addr(node->lhs);
        gen(node->rhs);
        store();
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    jmp .L.return.%s\n", g_funcname);
        return;
    case ND_IF:
    {
        int seq = g_labelseq++;
        if (node->els)
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .L.else.%d\n", seq);
            gen(node->then);
            printf("    jmp .L.end.%d\n", seq);
            printf(".L.else.%d:\n", seq);
            gen(node->els);
            printf(".L.end.%d:\n", seq);
        }
        else
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .L.end.%d\n", seq);
            gen(node->then);
            printf(".L.end.%d:\n", seq);
        }
        return;
    } // case ND_IF
    case ND_WHILE:
    {
        int seq = g_labelseq++;
        printf(".L.begin.%d:\n", seq);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je  .L.end.%d\n", seq);
        gen(node->then);
        printf("    jmp .L.begin.%d\n", seq);
        printf(".L.end.%d:\n", seq);
        return;
    } // case ND_WHILE
    case ND_FOR:
    {
        int seq = g_labelseq++;
        if (node->init)
        {
            gen(node->init);
        }
        printf(".L.begin.%d:\n", seq);
        if (node->cond)
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .L.end.%d\n", seq);
        }
        gen(node->then);
        if (node->inc)
        {
            gen(node->inc);
        }
        printf("    jmp .L.begin.%d\n", seq);
        printf(".L.end.%d:\n", seq);
        return;
    } // case ND_FOR
    case ND_BLOCK:
        for (Node *n = node->body; n; n = n->next)
        {
            gen(n);
        }
        return;
    case ND_FUNCALL:
    {
        int nargs = 0;
        for (Node *arg = node->args; arg; arg = arg->next)
        {
            gen(arg);
            nargs++;
        }

        for (int i = nargs - 1; i >= 0; i--)
        {
            printf("    pop %s\n", g_argreg[i]);
        }

        // x86-64の関数呼び出しのABIは、関数呼び出しをする前にRSPが16の倍数になっていなければならない
        int seq = g_labelseq++;
        printf("    mov rax, rsp\n");
        printf("    and rax, 15\n");
        printf("    jnz .L.call.%d\n", seq);
        printf("    mov rax, 0\n");
        printf("    call %s\n", node->funcname);
        printf("    jmp .L.end.%d\n", seq);
        printf(".L.call.%d:\n", seq);
        printf("    sub rsp, 8\n");
        printf("    mov rax, 0\n");
        printf("    call %s\n", node->funcname);
        printf("    add rsp, 8\n");
        printf(".L.end.%d:\n", seq);
        printf("    push rax\n");
        return;
    } // case ND_FUNCALL
    } // switch

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n"); // 比較命令の結果はフラグレジスタにセットされる
        printf("    sete al\n");      // cmp命令で調べた2つのレジスタの値が同じなら1、それ以外なら0
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

void codegen(Function *prog)
{
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    // 定義された関数ごとに出力
    for (Function *fn = prog; fn; fn = fn->next)
    {
        printf(".global %s\n", fn->name);
        printf("%s:\n", fn->name);
        g_funcname = fn->name;

        // プロローグ
        // 生成されたローカル変数分の領域を確保
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, %d\n", fn->stack_size);

        // 引数をスタックにプッシュ
        int i = 0;
        for (VarList *vl = fn->params; vl; vl = vl->next)
        {
            Var *var = vl->var;
            printf("    mov [rbp-%d], %s\n", var->offset, g_argreg[i++]);
        }

        // 抽象構文木を根から葉に下りながらコード生成
        for (Node *node = fn->node; node; node = node->next)
        {
            gen(node);
        }

        // エピローグ
        // 最後の式の結果が残っているのでそれが返り値になる
        printf(".L.return.%s:\n", fn->name);
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
    }
}