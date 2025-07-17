#include "lexer/lexer.h"     // lexer module, consider lazy lexing for large files
#include "parser/parser.h"   // parser module, might modularize further for expressions/statements
#include "sema/sema.h"       // semantic analysis, add type inference checks
#include "codegen/cgen.h"    // code generation, consider multiple backends
#include "utils/die.h"       // error handling, could support error codes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// reads the entire file into a buffer
// can add file size limit; mmap could improve performance
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) die("open %s", path);
    fseek(f, 0, SEEK_END); long sz = ftell(f); rewind(f);
    char *buf = malloc(sz+1); fread(buf, 1, sz, f); buf[sz]=0;
    fclose(f); return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) die("usage: kiloc <in.kl> [-o out.c]");  // basic arg check, could add --help
    const char *out = "out.c";                              // default output file
    if (argc >= 4 && !strcmp(argv[2], "-o")) out = argv[3]; // support for -o, extend to other flags

    char *src = read_file(argv[1]);     // load source file
    Lexer *L = lexer_new(src);          // init lexer, could cache tokens
    AST_Program *prog = parse(L);       // parse to ast, might log errors
    sema_check(prog);                   // run semantic analysis, should return status
    cgen_emit(prog, out);              // emit c code, could support ir dump
    return 0;                           // add proper exit codes on failure
}
