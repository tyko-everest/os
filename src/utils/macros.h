#ifndef INCLUDE_MACROS_H
#define INCLUDE_MACROS_H

// rounds the given number on specified interval
#define ROUND_DOWN(num, interval) (num / interval * interval)
#define ROUND_UP(num, interval) ((num / interval * interval) + (num % interval ? interval : 0))

#define CEIL(num, den) ((num / den) + (num % den ? 1 : 0))

#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

#endif