#ifndef INCLUDE_STRING_H
#define INCLUDE_STRING_H

#include "stdint.h"

typedef unsigned int size_t;

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *str, int c, size_t n);

#endif // INCLUDE_STRING_H