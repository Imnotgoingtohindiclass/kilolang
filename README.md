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

## Planned Extensions

| Feature   | Strategy                                                          |
| --------- | ----------------------------------------------------------------- |
| Floats    | Add `TOK_FLOAT`, `TYPE_FLOAT`, and `EXPR_FLOAT` variants          |
| Arrays    | Extend `Type` to support `TYPE_ARRAY(base, len)`, add `[]` syntax |
| LLVM IR   | Replace `codegen/cgen.c` with LLVM IR backend                     |
| REPL Mode | Evaluate statements by wrapping in a temporary `main` function    |

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE.md) for details.