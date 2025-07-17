#include "parser.h"
#include "../utils/die.h"
#include "../ast/ast.h"
#include <stdlib.h>
#include <string.h>

// parser struct holds lexer and current token
// could support peeking with token buffer
typedef struct {
    Lexer *l;
    Token cur;
} Parser;

// checks if current token matches kind
// may extend to accept multiple kinds
static inline bool match(Parser *p, TokenKind k) {
    return p->cur.kind == k;
}

// advance to next token
// assumes valid input; could add debug tracing
static inline void next(Parser *p) {
    p->cur = lexer_next(p->l);
}

// assert current token is expected kind
// gives error with line info
static void expect(Parser *p, TokenKind k) {
    if (!match(p, k))
        die("line %d: expected token kind %d, got %d",
            p->cur.line, k, p->cur.kind);
    next(p);
}

/* forward declarations */
// needed due to mutual recursion
static AST_Expr *parse_expr(Parser *p);
static AST_Block parse_block(Parser *p);

/* ------------------------------------------------------------------ */
/* helpers for AST lists */

// append stmt to block, grow dynamically
// realloc can be optimized with arena alloc
static void block_append(AST_Block *b, AST_Stmt *s) {
    if (b->count == b->cap) {
        b->cap = b->cap ? b->cap * 2 : 8;
        b->stmts = realloc(b->stmts, b->cap * sizeof *b->stmts);
    }
    b->stmts[b->count++] = s;
}

/* ------------------------------------------------------------------ */
/* type parsing */

// parses int or string types
// could support user-defined or array types
static Type parse_type(Parser *p) {
    if (match(p, TOK_INT)) {
        next(p);
        return TYPE_INT;
    }
    if (match(p, TOK_STRING)) {
        next(p);
        return TYPE_STRING;
    }
    die("line %d: type expected", p->cur.line);
    return TYPE_VOID;   // unreachable fallback
}

/* ------------------------------------------------------------------ */
/* expressions */

// create new expr of kind k
// calloc used to zero init
static AST_Expr *new_expr(ExprKind k) {
    AST_Expr *e = calloc(1, sizeof *e);
    e->kind = k;
    return e;
}

// parse literals, identifiers, calls, or parenthesized expressions
// no unary ops or indexing yet
static AST_Expr *parse_primary(Parser *p) {
    if (match(p, TOK_INT_LIT)) {
        AST_Expr *e = new_expr(EXPR_INT);
        e->int_lit = atoi(p->cur.text.p);
        next(p);
        return e;
    }
    if (match(p, TOK_STR_LIT)) {
        AST_Expr *e = new_expr(EXPR_STR);
        e->str_lit = strdup(p->cur.text.p);
        next(p);
        return e;
    }
    if (match(p, TOK_IDENT)) {
        const char *name = p->cur.text.p;
        next(p);
        if (match(p, TOK_LPAREN)) {
            // parse function call
            next(p);
            AST_Expr *e = new_expr(EXPR_CALL);
            e->call.name = strdup(name);
            e->call.args = NULL;
            e->call.arg_count = 0;

            if (!match(p, TOK_RPAREN)) {
                do {
                    // grow arg list
                    if (e->call.arg_count == 0)
                        e->call.args = malloc(sizeof *e->call.args);
                    else
                        e->call.args = realloc(e->call.args,
                            (e->call.arg_count + 1) * sizeof *e->call.args);
                    e->call.args[e->call.arg_count++] = parse_expr(p);
                } while (match(p, TOK_COMMA) && (next(p), 1));
            }
            expect(p, TOK_RPAREN);
            return e;
        } else {
            AST_Expr *e = new_expr(EXPR_IDENT);
            e->ident = strdup(name);
            return e;
        }
    }
    if (match(p, TOK_LPAREN)) {
        next(p);
        AST_Expr *e = parse_expr(p);
        expect(p, TOK_RPAREN);
        return e;
    }
    die("line %d: primary expected", p->cur.line);
    return NULL;
}

// parse comparison expressions
// left-associative; can extend precedence support
static AST_Expr *parse_cmp(Parser *p) {
    AST_Expr *left = parse_primary(p);
    while (match(p, TOK_EQ) || match(p, TOK_NE) || match(p, TOK_LT) ||
           match(p, TOK_LE) || match(p, TOK_GT) || match(p, TOK_GE)) {
        TokenKind op = p->cur.kind;
        next(p);
        AST_Expr *right = parse_primary(p);
        AST_Expr *e = new_expr(EXPR_CMP);
        e->cmp.left = left;
        e->cmp.right = right;
        e->cmp.cmp = op;
        left = e;
    }
    return left;
}

// parse full expression with + and -
static AST_Expr *parse_expr(Parser *p) {
    AST_Expr *left = parse_cmp(p);
    while (match(p, TOK_PLUS) || match(p, TOK_MINUS)) {
        TokenKind op = p->cur.kind;
        next(p);
        AST_Expr *right = parse_cmp(p);
        AST_Expr *e = new_expr(EXPR_BIN);
        e->bin.left = left;
        e->bin.right = right;
        e->bin.op = op;
        left = e;
    }
    return left;
}

/* ------------------------------------------------------------------ */
/* statements */

// alloc new stmt of kind k
static AST_Stmt *new_stmt(StmtKind k) {
    AST_Stmt *s = calloc(1, sizeof *s);
    s->kind = k;
    return s;
}

// parse var decl (manual or not)
// no const or mutability flags yet
static AST_VarDecl parse_vardecl(Parser *p) {
    AST_VarDecl vd = {0};
    if (match(p, TOK_MANUAL)) {
        vd.manual = true;
        next(p);
    }
    vd.type = parse_type(p);
    vd.name = strdup(p->cur.text.p);
    expect(p, TOK_IDENT);

    if (match(p, TOK_ASSIGN)) {
        next(p);
        vd.init = parse_expr(p);
    }
    expect(p, TOK_SEMI);
    return vd;
}

// parse one parameter (type + name)
static AST_Param parse_param(Parser *p) {
    AST_Param param = {0};
    param.type = parse_type(p);
    param.name = strdup(p->cur.text.p);
    expect(p, TOK_IDENT);
    return param;
}

// parse block of statements
// could track scope depth for nesting
static AST_Block parse_block(Parser *p) {
    AST_Block b = {0};
    expect(p, TOK_LBRACE);
    while (!match(p, TOK_RBRACE)) {
        AST_Stmt *s = NULL;
        if (match(p, TOK_INT) || match(p, TOK_STRING) || match(p, TOK_MANUAL)) {
            s = new_stmt(STMT_VAR);
            s->var = parse_vardecl(p);
        } else if (match(p, TOK_IDENT)) {
            // parse assignment
            const char *name = p->cur.text.p;
            next(p);
            expect(p, TOK_ASSIGN);
            s = new_stmt(STMT_ASSIGN);
            s->assign.name = strdup(name);
            s->assign.expr = parse_expr(p);
            expect(p, TOK_SEMI);
        } else if (match(p, TOK_IF)) {
            next(p);
            expect(p, TOK_LPAREN);
            s = new_stmt(STMT_IF);
            s->if_.cond = parse_expr(p);
            expect(p, TOK_RPAREN);
            s->if_.then = parse_block(p);
            s->if_.else_.count = 0;
            s->if_.else_.stmts = NULL;
            if (match(p, TOK_ELSE)) {
                next(p);
                s->if_.else_ = parse_block(p);
            }
        } else if (match(p, TOK_WHILE)) {
            next(p);
            expect(p, TOK_LPAREN);
            s = new_stmt(STMT_WHILE);
            s->while_.cond = parse_expr(p);
            expect(p, TOK_RPAREN);
            s->while_.body = parse_block(p);
        } else if (match(p, TOK_PRINT)) {
            next(p);
            expect(p, TOK_LPAREN);
            s = new_stmt(STMT_PRINT);
            s->print = parse_expr(p);
            expect(p, TOK_RPAREN);
            expect(p, TOK_SEMI);
        } else if (match(p, TOK_RETURN)) {
            next(p);
            s = new_stmt(STMT_RETURN);
            s->ret = NULL;
            if (!match(p, TOK_SEMI))
                s->ret = parse_expr(p);
            expect(p, TOK_SEMI);
        } else {
            die("line %d: statement expected", p->cur.line);
        }
        block_append(&b, s);
    }
    expect(p, TOK_RBRACE);
    return b;
}

// parse function definition
// could validate duplicate names here
static AST_FuncDecl parse_func(Parser *p) {
    AST_FuncDecl f = {0};
    expect(p, TOK_FUNC);
    f.name = strdup(p->cur.text.p);
    expect(p, TOK_IDENT);

    expect(p, TOK_LPAREN);
    if (!match(p, TOK_RPAREN)) {
        do {
            if (f.param_count == 0)
                f.params = malloc(sizeof *f.params);
            else
                f.params = realloc(f.params,
                        (f.param_count + 1) * sizeof *f.params);
            f.params[f.param_count++] = parse_param(p);
        } while (match(p, TOK_COMMA) && (next(p), 1));
    }
    expect(p, TOK_RPAREN);

    expect(p, TOK_ARROW);
    f.ret_ty = parse_type(p);
    f.body = parse_block(p);
    return f;
}

/* ------------------------------------------------------------------ */
/* entry point */

// parse entire program
// assumes only function-level top decls
AST_Program *parse(Lexer *l) {
    Parser p = { .l = l, .cur = lexer_next(l) };

    AST_Program *prog = malloc(sizeof *prog);
    prog->funcs = NULL;
    prog->func_count = 0;
    prog->cap = 0;

    while (!match(&p, TOK_EOF)) {
        if (prog->func_count == prog->cap) {
            prog->cap = prog->cap ? prog->cap * 2 : 8;
            prog->funcs = realloc(prog->funcs,
                                  prog->cap * sizeof *prog->funcs);
        }
        prog->funcs[prog->func_count++] = parse_func(&p);
    }
    return prog;
}
