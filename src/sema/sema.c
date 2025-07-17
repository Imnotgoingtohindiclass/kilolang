#include "sema.h"
#include "../utils/die.h"
#include <string.h>
#include <stdlib.h>

// semantic state holder
// could move to context struct for multithreaded use
typedef struct {
    AST_FuncDecl *funcs;
    int func_count;
    const char **locals;
    Type *local_ty;
    bool *local_manual;
    int local_count;
} Sema;

static Sema g;  // global sema context, should reset before reuse

// find func index by name
// could cache results if funcs are many
static int find_func(const char *name) {
    for (int i=0;i<g.func_count;i++) if (!strcmp(g.funcs[i].name, name)) return i;
    return -1;
}

// find local var index by name
// might switch to hash table if many locals
static int find_local(const char *name) {
    for (int i=0;i<g.local_count;i++) if (!strcmp(g.locals[i], name)) return i;
    return -1;
}

// infer type of expression
// doesn't handle type coercion or overloads
static Type expr_type(AST_Expr *e) {
    switch (e->kind) {
    case EXPR_INT: return TYPE_INT;
    case EXPR_STR: return TYPE_STRING;
    case EXPR_IDENT: {
        int idx = find_local(e->ident);
        if (idx==-1) die("undefined var %s", e->ident);  // no forward ref
        return g.local_ty[idx];
    }
    case EXPR_BIN: {
        Type l = expr_type(e->bin.left), r = expr_type(e->bin.right);
        if (l!=TYPE_INT || r!=TYPE_INT) die("bin op type");  // strict typing
        return TYPE_INT;
    }
    case EXPR_CMP: {
        Type l = expr_type(e->cmp.left), r = expr_type(e->cmp.right);
        if (l!=TYPE_INT || r!=TYPE_INT) die("cmp op type");
        return TYPE_INT;
    }
    case EXPR_CALL: {
        int f = find_func(e->call.name);
        if (f==-1) die("unknown func %s", e->call.name);  // no forward decls
        return g.funcs[f].ret_ty;
    }
    }
    return TYPE_VOID;  // fallback, unreachable if exhaustive
}

// check block semantics
// doesn't track unreachable code or dead vars
static void check_block(AST_Block b, Type ret_ty) {
    for (int i=0;i<b.count;i++) {
        AST_Stmt *s = b.stmts[i];
        switch (s->kind) {
        case STMT_VAR: {
            if (find_local(s->var.name)!=-1) die("redef var %s", s->var.name);
            int i = g.local_count++;
            g.locals = realloc(g.locals, g.local_count*sizeof(char*));       // no null check
            g.local_ty = realloc(g.local_ty, g.local_count*sizeof(Type));
            g.local_manual = realloc(g.local_manual, g.local_count*sizeof(bool));
            g.locals[i] = s->var.name;
            g.local_ty[i] = s->var.type;
            g.local_manual[i] = s->var.manual;
            if (s->var.init) {
                Type t = expr_type(s->var.init);
                if (t != s->var.type) die("var init type");  // strict match
            }
            break;
        }
        case STMT_ASSIGN: {
            int idx = find_local(s->assign.name);
            if (idx==-1) die("assign undef %s", s->assign.name);
            Type t = expr_type(s->assign.expr);
            if (t != g.local_ty[idx]) die("assign type");
            break;
        }
        case STMT_IF:
            if (expr_type(s->if_.cond) != TYPE_INT) die("if cond type");
            check_block(s->if_.then, ret_ty);
            if (s->if_.else_.count) check_block(s->if_.else_, ret_ty);
            break;
        case STMT_WHILE:
            if (expr_type(s->while_.cond) != TYPE_INT) die("while cond type");
            check_block(s->while_.body, ret_ty);
            break;
        case STMT_PRINT:
            expr_type(s->print);  // just check it's valid
            break;
        case STMT_RETURN:
            if (s->ret) {
                Type t = expr_type(s->ret);
                if (t != ret_ty) die("return type");
            }
            break;
        }
    }
}

// entry point for semantic analysis
// could separate func decl pass and body check pass
void sema_check(AST_Program *p) {
    g.funcs = p->funcs;
    g.func_count = p->func_count;
    g.locals = NULL; g.local_count = 0;
    if (find_func("main")==-1) die("no main");  // ensure entry point
    for (int i=0;i<g.func_count;i++) {
        AST_FuncDecl *f = &g.funcs[i];
        check_block(f->body, f->ret_ty);  // validate body
    }
}