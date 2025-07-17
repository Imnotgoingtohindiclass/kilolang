# KiloLang Micro-Compiler

**"A tiny, safe, garbage-collected C-style language"**

## ⚠️ THIS MICRO-COMPILER IS UNDER PRODUCTION AND DOES NOT FULLY WORK. USE AT YOUR OWN RISK ⚠️

---

## What is KiloLang?

KiloLang is a hand-written, single-pass compiler for a miniature, statically-typed, C-like language. It generates standard C11 source code that can be compiled with any conforming C compiler such as `gcc`, `clang`, or `tcc`.

| Feature                   | Status                 |
| ------------------------- | ---------------------- |
| Lexer & Parser            | ✅                      |
| Semantic Checker          | ✅                      |
| C Code Generator          | ✅                      |
| Mark & Sweep GC           | ✅                      |
| Manual Memory Opt-Out     | ✅ (`manual` keyword)   |
| Arrays / Structs / Floats | ❌ (planned extensions) |

---

## Architecture

```
kilolang/
├── src/
│   ├── lexer/        // UTF-8 safe DFA scanner
│   ├── parser/       // recursive-descent, Pratt-ready
│   ├── ast/          // arena-backed AST node pool
│   ├── sema/         // symbol table and type checker
│   ├── codegen/      // naïve C11 emitter
│   ├── gc/           // stop-the-world mark & sweep collector
│   └── utils/        // strbuf, arena, error handling
├── examples/         // sample .kl programs
├── Makefile
└── README.md         // this masterpiece
```

---

## Quick Start

```bash
# 1. Build
make clean && make          # → bin/kiloc

# 2. Compile a KiloLang program
bin/kiloc examples/demo.kl -o demo.c

# 3. Compile the generated C code
cc demo.c src/gc/gc.c -o demo

# 4. Run the binary
./demo
```

Or download the kiloc file and run the binary directly without compiling it (lazy bum)
---

## Example Program

```c
func add(int x, int y) -> int {
    return x + y;
}

func main() -> int {
    int  n   = 5;
    manual string msg = "Hello, world!";
    print(add(n, 7));
    print(msg);
    free(msg);          // manual memory
    return 0;
}
```

---

## Language Specification

| Category  | Description                                                                       |
| --------- | --------------------------------------------------------------------------------- |
| Types     | `int`, `string`, `void`                                                           |
| Storage   | `T name = val;` for GC-managed memory<br>`manual T name = val;` for manual `free` |
| Control   | `if`, `else`, `while`, `return`, `print(expr)`                                    |
| Operators | `+ - * / == != < <= > >=`                                                         |
| Functions | No overloading, single return value only                                          |

---

## Known Issues and Fixes

| Date       | Issue                         | Cause                            | Fix                                      |
| ---------- | ----------------------------- | -------------------------------- | ---------------------------------------- |
| 2024-07-17 | `strcmp` undefined            | Missing `<string.h>`             | Added to `main.c`                        |
| 2024-07-17 | `malloc`, `realloc` undefined | Missing `<stdlib.h>`             | Added to `lexer.c`, `sema.c`, `parser.c` |
| 2024-07-17 | Duplicate `case '-'` in lexer | Redundant lexer cases            | Merged the blocks                        |
| 2024-07-17 | `bool` undefined              | Missing `<stdbool.h>`            | Added to `lexer.c`                       |
| 2024-07-17 | Heap corruption error         | `make_tok()` reused freed memory | Lexer restructured                       |

---

## Heap Corruption Debug Log

### Error Message

```
malloc: Heap corruption detected, free list is damaged
```

### Root Cause

`make_tok()` returned a token by value, but the `text` field was initialized with a reference to already-advanced `l->start`, which had been freed or reallocated. This caused `StrBuf` to reference stale memory, resulting in double-free or use-after-free.

### Fix

* `lexer_next()` now returns tokens with a copy of the lexeme.
* `StrBuf` initialized once per token.
* No aliasing of internal lexer buffers anymore.

---

## Planned Extensions

| Feature   | Strategy                                                          |
| --------- | ----------------------------------------------------------------- |
| Floats    | Add `TOK_FLOAT`, `TYPE_FLOAT`, and `EXPR_FLOAT` variants          |
| Arrays    | Extend `Type` to support `TYPE_ARRAY(base, len)`, add `[]` syntax |
| LLVM IR   | Replace `codegen/cgen.c` with LLVM IR backend                     |
| REPL Mode | Evaluate statements by wrapping in a temporary `main` function    |

---

## License

Public Domain — free to use, modify, and redistribute.