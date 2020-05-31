#ifndef INCLUDE_STDIO_H
#define INCLUDE_STDIO_H

#include "stdarg.h"
#include "string.h"

#include "keyboard.h"
#include "print.h"

int putchar(int c);
int puts(const char *str);
int printf(const char *format, ...);
int scanf(const char *format, ...);

#endif // INCLUDE_STDIO_H