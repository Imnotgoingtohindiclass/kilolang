#include "strbuf.h"
#include <stdlib.h>
#include <string.h>

// init empty buffer
// maybe reserve small cap by default to avoid early realloc
void strbuf_init(StrBuf *sb) { sb->p = NULL; sb->len = sb->cap = 0; }

// append string to buffer
// realloc policy can lead to waste; consider exact fit or amortized growth
void strbuf_append(StrBuf *sb, const char *s) {
    size_t n = strlen(s);
    if (sb->len + n + 1 > sb->cap) {
        sb->cap = sb->cap ? sb->cap * 2 : 64;
        sb->p = realloc(sb->p, sb->cap);  // no null check; can fail silently
    }
    memcpy(sb->p + sb->len, s, n);
    sb->len += n;
    sb->p[sb->len] = '\0';  // ensures null-termination
}

// get c-string view
// returns empty string if buffer not initialized
const char *strbuf_cstr(StrBuf *sb) { return sb->p ? sb->p : ""; }
