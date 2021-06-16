#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#include "clib/stddef.h"
#include "clib/string.h"
#include "system/kheap.h"
#include "system/proc.h"
#include "system/vfs.h"

typedef enum {
    SYS_READ    = 0,
    SYS_WRITE   = 1,
    SYS_MAKE    = 2,
    SYS_DELETE  = 3,

    SYS_BRK     = 10,

    SYS_FORK    = 20,
    SYS_EXEC    = 21,
    SYS_EXIT    = 22,

    SYS_PRINT   = 30,
    SYS_INPUT   = 31
} syscall_no_t;

ssize_t sys_read(const char *path, void *buf, size_t count, size_t from);
ssize_t sys_write(const char *path, void *buf, size_t count, size_t from);
ssize_t sys_make(const char *path, bool is_dir);
ssize_t sys_delete(const char *path);

void sys_exec(const char *path);

void sys_print(const char *str);

#endif
