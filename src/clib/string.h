#ifndef INCLUDE_STRING_H
#define INCLUDE_STRING_H

#include "stddef.h"
#include "stdint.h"

typedef unsigned int size_t;

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *str, int c, size_t n);
int memcmp(const void *str1, const void *str2, size_t n);

size_t strlen(const char *str);
char *strchr(const char *str, int c);
char *strtok(char *str, const char *delim);
int strcmp (const char* str1, const char* str2);

#endif // INCLUDE_STRING_H