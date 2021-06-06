#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#include "clib/stddef.h"
#include "system/fat32.h"

typedef enum {
    SYS_READ    = 0,
    SYS_WRITE   = 1,
    SYS_MAKE    = 2,
    SYS_DELETE  = 3,

    SYS_BRK     = 10,

    SYS_FORK    = 20,
    SYS_EXEC    = 21,
    SYS_EXIT    = 22,

    SYS_PRINT   = 30
} syscall_no_t;

ssize_t sys_read(const char *path, void *buf, size_t count, size_t from);
void sys_print(const char *str);

#endif
