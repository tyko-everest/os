#ifndef INCLUDE_STDARG_H
#define INCLUDE_STDARG_H

#include "macros.h"

typedef __builtin_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)

#endif // INCLUDE_STDARG_H