#include "9cc.h"

// 抽象構文木をアセンブリに変換
static void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %ld\n", node->val);
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    ret\n");
        return;
    }

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

void codegen(Node *node)
{
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    for (Node *n = node; n; n = n->next)
    {
        gen(n);
        // スタックトップに式全体の値が残っているはずなので、それをRAXにロード
        printf("    pop rax\n");
    }

    printf("    ret\n");
}