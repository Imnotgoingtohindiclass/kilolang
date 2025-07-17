#pragma once
#include "../lexer/lexer.h"
#include "../ast/ast.h"
AST_Program *parse(Lexer *l);