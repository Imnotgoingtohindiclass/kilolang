#include "ast.h"
#include <stdlib.h>

// create empty program node
AST_Program *ast_program_new(void) {
    AST_Program *p = calloc(1, sizeof *p);
    return p; // maybe add error check for calloc fail
}

// add function to program
void ast_add_func(AST_Program *p, AST_FuncDecl f) {
    if (p->func_count == p->cap) {
        p->cap = p->cap ? p->cap*2 : 8;
        p->funcs = realloc(p->funcs, p->cap * sizeof *p->funcs);
        // could wrap realloc with check and grow helper
    }
    p->funcs[p->func_count++] = f;
}