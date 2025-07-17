#include "lexer.h"
#include "../utils/die.h"
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// check if a char is valid in an identifier
static bool isident(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

// keyword lookup table, index is token enum
static const char *keywords[] = {
    [TOK_FUNC]   = "func",
    [TOK_IF]     = "if",
    [TOK_ELSE]   = "else",
    [TOK_WHILE]  = "while",
    [TOK_PRINT]  = "print",
    [TOK_RETURN] = "return",
    [TOK_INT]    = "int",
    [TOK_STRING] = "string",
    [TOK_MANUAL] = "manual",
};

// allocate and initalise lexer
Lexer *lexer_new(const char *src) {
    Lexer *l = malloc(sizeof *l);
    l->start = l->cur = src;
    l->line = 1;
    return l;
}

// make a token from current range; note: text capture is broken
static Token make_token(Lexer *l, TokenKind kind,
                        const char *start, const char *end) {
    Token t = { .kind = kind, .line = l->line };
    strbuf_init(&t.text);
    strbuf_append(&t.text, strndup(start, (size_t)(end - start)));
    return t;
}

// check if identifier matches a keyword
static TokenKind ident_kind(const char *s, size_t len) {
    for (int k = TOK_FUNC; k <= TOK_MANUAL; ++k)
        if (keywords[k] && strncmp(s, keywords[k], len) == 0)
            return k;
    return TOK_IDENT;
}

// lex on token from input stream
Token lexer_next(Lexer *l) {
    for (;;) {
        l->start = l->cur;
        switch (*l->cur) {
        case '\0':
            return make_token(l, TOK_EOF, l->cur, l->cur + 1);

        // skip whitespace and comments
        case ' ': case '\t':
            ++l->cur; continue;
        case '\n':
            ++l->line; ++l->cur; continue;
        case '/':
            if (l->cur[1] == '/') {
                l->cur += 2;
                while (*l->cur && *l->cur != '\n') ++l->cur;
                continue;
            }
            ++l->cur;
            return make_token(l, TOK_SLASH, l->start, l->cur);

        // prase integer literals
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            while (isdigit((unsigned char)*l->cur)) ++l->cur;
            return make_token(l, TOK_INT_LIT, l->start, l->cur);

        // parse string literals
        case '"': {
            const char *start = ++l->cur;
            while (*l->cur && *l->cur != '"') {
                if (*l->cur == '\n') ++l->line;
                ++l->cur;
            }
            if (*l->cur != '"') die("line %d: unterminated string", l->line);
            Token t = make_token(l, TOK_STR_LIT, start, l->cur);
            ++l->cur;                       /* skip closing '"' */
            return t;
        }

        // parse identifiers/keywords
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_': {
            while (isident(*l->cur)) ++l->cur;
            TokenKind k = ident_kind(l->start, (size_t)(l->cur - l->start));
            return make_token(l, k, l->start, l->cur);
        }

        // parse simple and compound operators
        case '-':
            if (l->cur[1] == '>') { l->cur += 2; return make_token(l, TOK_ARROW, l->start, l->cur); }
            ++l->cur; return make_token(l, TOK_MINUS, l->start, l->cur);

        case '=':
            if (l->cur[1] == '=') { l->cur += 2; return make_token(l, TOK_EQ, l->start, l->cur); }
            ++l->cur; return make_token(l, TOK_ASSIGN, l->start, l->cur);

        case '!':
            if (l->cur[1] == '=') { l->cur += 2; return make_token(l, TOK_NE, l->start, l->cur); }
            die("line %d: '!' must be followed by '='", l->line);

        case '<':
            if (l->cur[1] == '=') { l->cur += 2; return make_token(l, TOK_LE, l->start, l->cur); }
            ++l->cur; return make_token(l, TOK_LT, l->start, l->cur);

        case '>':
            if (l->cur[1] == '=') { l->cur += 2; return make_token(l, TOK_GE, l->start, l->cur); }
            ++l->cur; return make_token(l, TOK_GT, l->start, l->cur);

        /* single-char tokens */
        case '+': ++l->cur; return make_token(l, TOK_PLUS, l->start, l->cur);
        case '*': ++l->cur; return make_token(l, TOK_STAR, l->start, l->cur);
        case ';': ++l->cur; return make_token(l, TOK_SEMI, l->start, l->cur);
        case ',': ++l->cur; return make_token(l, TOK_COMMA, l->start, l->cur);
        case '(': ++l->cur; return make_token(l, TOK_LPAREN, l->start, l->cur);
        case ')': ++l->cur; return make_token(l, TOK_RPAREN, l->start, l->cur);
        case '{': ++l->cur; return make_token(l, TOK_LBRACE, l->start, l->cur);
        case '}': ++l->cur; return make_token(l, TOK_RBRACE, l->start, l->cur);

        default:
            die("line %d: unexpected character '%c'", l->line, *l->cur);
        }
    }
}