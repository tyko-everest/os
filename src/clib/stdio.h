#ifndef INCLUDE_STDIO_H
#define INCLUDE_STDIO_H

#include "clib/stdarg.h"
#include "clib/string.h"

#define EOF (-1)

// helper functions
static int int2str(int num, int count);

// public functions
int putchar(int c);
int puts(const char *str);
int printf(const char *format, ...);
int scanf(const char *format, ...);

#endif // INCLUDE_STDIO_H