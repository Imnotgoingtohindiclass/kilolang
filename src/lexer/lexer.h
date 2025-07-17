#pragma once  // avoid multiple inclusion

#include "../utils/strbuf.h"  // for token text buffer

// all possible token types
// can be extended for other operators or keywords
typedef enum {
    TOK_EOF, TOK_FUNC, TOK_IF, TOK_ELSE, TOK_WHILE,
    TOK_PRINT, TOK_RETURN,
    TOK_INT, TOK_STRING, TOK_MANUAL,
    TOK_IDENT, TOK_INT_LIT, TOK_STR_LIT,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH,
    TOK_EQ, TOK_NE, TOK_LT, TOK_LE, TOK_GT, TOK_GE,
    TOK_ASSIGN, TOK_SEMI, TOK_COMMA,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_ARROW,
} TokenKind;

// single token structure
// consider adding column info for better error messages
typedef struct {
    TokenKind kind;
    StrBuf text;
    int line;
} Token;

// lexer state
// can add filename for multi-file support
typedef struct {
    const char *start, *cur;
    int line;
} Lexer;

Lexer *lexer_new(const char *src);  // create new lexer
Token lexer_next(Lexer *l);         // return next token