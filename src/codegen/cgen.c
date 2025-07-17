#include "cgen.h"
#include "../utils/die.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* token constants we need */
#define TOK_PLUS 1
#define TOK_MINUS 2
#define TOK_EQ 3
#define TOK_NE 4
#define TOK_LT 5
#define TOK_LE 6
#define TOK_GT 7
#define TOK_GE 8

/* get expr type – sema has real info, this is for printing only */
static Type expr_type(AST_Expr *e) {
    switch (e->kind) {
    case EXPR_INT: return TYPE_INT;
    case EXPR_STR: return TYPE_STRING;
    case EXPR_IDENT: return TYPE_INT; /* assume int – use sema later */
    case EXPR_BIN:   return TYPE_INT;
    case EXPR_CMP:   return TYPE_INT;
    case EXPR_CALL:  return TYPE_INT; /* assume int – not always true */
    }
    return TYPE_VOID;
}

static FILE *out;

/* wrapper around printf for output */
static void emit(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); vfprintf(out, fmt, ap); va_end(ap);
}

/* map ir type to c type string */
static const char *ctype(Type t) {
    switch (t) {
    case TYPE_INT: return "int";
    case TYPE_STRING: return "char*";
    default: return "void"; // todo: handle other types later
    }
}

/* generate code for expression */
static void expr_gen(AST_Expr *e) {
    switch (e->kind) {
    case EXPR_INT: emit("%d", e->int_lit); break;
    case EXPR_STR: emit("\"%s\"", e->str_lit); break;
    case EXPR_IDENT: emit("%s", e->ident); break;
    case EXPR_BIN:
        emit("("); expr_gen(e->bin.left);
        emit(" %c ", e->bin.op==TOK_PLUS?'+':'-'); // todo: handle * / %
        expr_gen(e->bin.right); emit(")"); break;
    case EXPR_CMP: {
        expr_gen(e->cmp.left);
        const char *ops[] = { "==", "!=", "<", "<=", ">", ">=" };
        emit(" %s ", ops[e->cmp.cmp - TOK_EQ]); // fragile – validate enum alignment
        expr_gen(e->cmp.right); break;
    }
    case EXPR_CALL:
        emit("%s(", e->call.name);
        for (int i=0;i<e->call.arg_count;i++) {
            if (i) emit(", ");
            expr_gen(e->call.args[i]);
        }
        emit(")"); break;
    }
}

/* generate code for statements */
static void stmt_gen(AST_Stmt *s) {
    switch (s->kind) {
    case STMT_VAR: {
        emit("    %s %s", ctype(s->var.type), s->var.name);
        if (s->var.init) { emit(" = "); expr_gen(s->var.init); }
        emit(";\n");
        break;
    }
    case STMT_ASSIGN:
        emit("    %s = ", s->assign.name); expr_gen(s->assign.expr); emit(";\n"); break;
    case STMT_IF:
        emit("    if ("); expr_gen(s->if_.cond); emit(") {\n");
        for (int i=0;i<s->if_.then.count;i++) stmt_gen(s->if_.then.stmts[i]);
        emit("    }\n");
        if (s->if_.else_.count) {
            emit("    else {\n");
            for (int i=0;i<s->if_.else_.count;i++) stmt_gen(s->if_.else_.stmts[i]);
            emit("    }\n");
        }
        break;
    case STMT_WHILE:
        emit("    while ("); expr_gen(s->while_.cond); emit(") {\n");
        for (int i=0;i<s->while_.body.count;i++) stmt_gen(s->while_.body.stmts[i]);
        emit("    }\n");
        break;
    case STMT_PRINT:
        emit("    printf(\"%s\\n\", ", 
             expr_type(s->print)==TYPE_INT ? "%d" : "%s"); // todo: support more types
        expr_gen(s->print); emit(");\n");
        break;
    case STMT_RETURN:
        emit("    return"); if (s->ret) { emit(" "); expr_gen(s->ret); } emit(";\n");
        break;
    }
}

/* main codegen entry – emits full c file */
void cgen_emit(AST_Program *p, const char *outfile) {
    out = fopen(outfile, "w");
    if (!out) die("open %s", outfile); // todo: better error message
    emit("#include <stdio.h>\n");
    emit("#include \"gc.h\"\n\n"); // todo: maybe conditional include if gc used
    for (int i=0;i<p->func_count;i++) {
        AST_FuncDecl *f = &p->funcs[i];
        emit("int %s(", f->name);
        for (int j=0;j<f->param_count;j++) {
            if (j) emit(", ");
            emit("%s %s", ctype(f->params[j].type), f->params[j].name);
        }
        emit(") {\n");
        for (int j=0;j<f->body.count;j++) stmt_gen(f->body.stmts[j]);
        emit("    return 0;\n}\n\n"); // todo: handle non-int return types
    }
    fclose(out);
}