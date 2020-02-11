#include "9cc.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    g_user_input = argv[1];
    g_token = tokenize(argv[1]);
    Function *prog = program();

    // ローカル変数群にRBPからのオフセットを割り当てる
    for (Function *fn = prog; fn; fn = fn->next)
    {
        int offset = 0;
        for (VarList *vl = fn->locals; vl; vl = vl->next)
        {
            offset += 8;
            vl->var->offset = offset;
        }
        fn->stack_size = offset;
    }

    codegen(prog);

    return 0;
}