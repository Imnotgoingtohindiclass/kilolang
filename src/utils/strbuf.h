#pragma once  // prevent multiple inclusion

#include <stddef.h>  // for size_t

// dynamic string buffer
// can add support for appending chars, freeing memory, resizing policy
typedef struct { char *p; size_t len, cap; } StrBuf;

void strbuf_init(StrBuf *sb);                  // init buffer, could allow initial cap
void strbuf_append(StrBuf *sb, const char *s); // append string, may need realloc strategy
const char *strbuf_cstr(StrBuf *sb);           // get null-terminated string, ensure null termination
