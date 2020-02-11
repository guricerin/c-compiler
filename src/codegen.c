#include "9cc.h"

// アセンブリにおけるラベルの通し番号
static int labelseq = 1;

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
        printf("    jmp .L.return\n");
        return;
    case ND_IF:
    {
        int seq = labelseq++;
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
        int seq = labelseq++;
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
        int seq = labelseq++;
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
        printf("    call %s\n", node->funcname);
        printf("    push rax\n");
        return;
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
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 生成されたローカル変数分の領域を確保
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", prog->stack_size);

    // 抽象構文木を根から葉に下りながらコード生成
    for (Node *node = prog->node; node; node = node->next)
    {
        gen(node);
    }

    // エピローグ
    // 最後の式の結果が残っているのでそれが返り値になる
    printf(".L.return:\n");
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}