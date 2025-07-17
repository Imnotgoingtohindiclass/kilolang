#pragma once
#include "../common.h"

// forward decls
typedef struct AST_Expr AST_Expr;
typedef struct AST_Stmt AST_Stmt;

// block of statements
typedef struct AST_Block {
    AST_Stmt **stmts;
    int count, cap;
} AST_Block; // maybe use dynamic array or arena for better perf

// variable declaration node
typedef struct {
    Type type;
    bool manual; // mark for manual memory mgmt
    const char *name;
    AST_Expr *init;
} AST_VarDecl; // add const/readonly flags later

// function param node
typedef struct {
    Type type;
    const char *name;
} AST_Param; // can add default values support later

// function decl node
typedef struct AST_FuncDecl {
    const char *name;
    AST_Param *params;
    int param_count;
    Type ret_ty;
    AST_Block body;
} AST_FuncDecl; // need support for extern funcs and attributes

// full program node
typedef struct AST_Program {
    AST_FuncDecl *funcs;
    int func_count, cap;
} AST_Program; // should support global vars too

/* expressions */

// kinds of expressions
typedef enum {
    EXPR_INT, EXPR_STR, EXPR_IDENT, EXPR_BIN, EXPR_CMP, EXPR_CALL
} ExprKind; // add unary ops and member access later

// expression node
struct AST_Expr {
    ExprKind kind;
    union {
        int int_lit;
        const char *str_lit;
        const char *ident;
        struct { int op; AST_Expr *left, *right; } bin;
        struct { int cmp; AST_Expr *left, *right; } cmp;
        struct { const char *name; AST_Expr **args; int arg_count; } call;
    };
}; // maybe add location info for better error reporting

/* statements */

// kinds of statements
typedef enum {
    STMT_VAR, STMT_ASSIGN, STMT_IF, STMT_WHILE, STMT_PRINT, STMT_RETURN
} StmtKind; // later: add for loop, break, continue, block

// statement node
struct AST_Stmt {
    StmtKind kind;
    union {
        AST_VarDecl var;
        struct { const char *name; AST_Expr *expr; } assign;
        struct { AST_Expr *cond; AST_Block then, else_; } if_;
        struct { AST_Expr *cond; AST_Block body; } while_;
        AST_Expr *print;
        AST_Expr *ret;
    };
}; // consider location tracking for error messages