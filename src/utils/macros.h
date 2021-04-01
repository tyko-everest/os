#ifndef INCLUDE_MACROS_H
#define INCLUDE_MACROS_H

// rounds the given number on specified interval
#define ROUND_DOWN(x, y) (x / y * y)
#define ROUND_UP(x, y) ((x / y * y) + (x % y ? y : 0))

#define CEIL(x, y) ((x / y) + (x % y ? 1 : 0))

#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

#endif