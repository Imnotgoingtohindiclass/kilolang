#include "lexer.h"
#include "../utils/die.h"
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

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

// allocate and initialize lexer
Lexer *lexer_new(const char *src) {
    Lexer *l = malloc(sizeof *l);
    l->start = l->cur = src;
    l->line = 1;
    return l;
}

// make a token from current range; note: text capture is broken
static Token make_tok(Lexer *l, TokenKind k) {
    Token t = { .kind = k, .line = l->line };
    strbuf_init(&t.text);
    strbuf_append(&t.text, l->start);  // fix: should use strbuf_appendn with (l->cur - l->start)
    l->start = l->cur;
    return t;
}

// check if identifier matches a keyword
static TokenKind ident_kind(Lexer *l) {
    for (int k = TOK_FUNC; k <= TOK_MANUAL; ++k)
        if (keywords[k] && strncmp(l->start, keywords[k], (size_t)(l->cur - l->start)) == 0)
            return k;
    return TOK_IDENT;
}

// lex one token from input stream
Token lexer_next(Lexer *l) {
    for (;;) {
        l->start = l->cur;
        switch (*l->cur) {
        case '\0': return make_tok(l, TOK_EOF);

        // skip whitespace and comments
        case ' ':
        case '\t':
            ++l->cur;
            continue;
        case '\n':
            ++l->line;
            ++l->cur;
            continue;
        case '/':
            if (l->cur[1] == '/') {
                l->cur += 2;
                while (*l->cur && *l->cur != '\n') ++l->cur;
                continue;
            }
            ++l->cur;
            return make_tok(l, TOK_SLASH);

        // parse integer literals
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            while (isdigit((unsigned char)*l->cur)) ++l->cur;
            return make_tok(l, TOK_INT_LIT);

        // parse string literals
        case '"':
            ++l->cur;
            l->start = l->cur;
            while (*l->cur && *l->cur != '"') {
                if (*l->cur == '\n') ++l->line;
                ++l->cur;
            }
            if (*l->cur != '"') die("unterminated string");
            Token t = make_tok(l, TOK_STR_LIT);
            ++l->cur;
            return t;

        // parse identifiers or keywords
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_':
            while (isident(*l->cur)) ++l->cur;
            return make_tok(l, ident_kind(l));

        // parse simple and compound operators
        case '+': ++l->cur; return make_tok(l, TOK_PLUS);
        case '*': ++l->cur; return make_tok(l, TOK_STAR);

        case '-':
            if (l->cur[1] == '>') {
                l->cur += 2;
                return make_tok(l, TOK_ARROW);
            }
            ++l->cur;
            return make_tok(l, TOK_MINUS);

        case '=':
            if (l->cur[1] == '=') {
                l->cur += 2;
                return make_tok(l, TOK_EQ);
            }
            ++l->cur;
            return make_tok(l, TOK_ASSIGN);

        case '!':
            if (l->cur[1] == '=') {
                l->cur += 2;
                return make_tok(l, TOK_NE);
            }
            die("'!' must be followed by '='");  // could allow '!' in future

        case '<':
            if (l->cur[1] == '=') {
                l->cur += 2;
                return make_tok(l, TOK_LE);
            }
            ++l->cur;
            return make_tok(l, TOK_LT);

        case '>':
            if (l->cur[1] == '=') {
                l->cur += 2;
                return make_tok(l, TOK_GE);
            }
            ++l->cur;
            return make_tok(l, TOK_GT);

        // punctuation and grouping
        case ';': ++l->cur; return make_tok(l, TOK_SEMI);
        case ',': ++l->cur; return make_tok(l, TOK_COMMA);
        case '(': ++l->cur; return make_tok(l, TOK_LPAREN);
        case ')': ++l->cur; return make_tok(l, TOK_RPAREN);
        case '{': ++l->cur; return make_tok(l, TOK_LBRACE);
        case '}': ++l->cur; return make_tok(l, TOK_RBRACE);

        // error for unknown character
        default:
            die("bad character '%c'", *l->cur);  // could improve with line/column
        }
    }
}
