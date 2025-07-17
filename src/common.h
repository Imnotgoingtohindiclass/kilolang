#pragma once  // standard guard

#include <stdint.h>   // fixed-width integer types
#include <stdbool.h>  // bool support, needed for c99+

// enum for basic types
// can add float, bool, custom types later
typedef enum {
    TYPE_INT, TYPE_STRING, TYPE_VOID
} Type;
