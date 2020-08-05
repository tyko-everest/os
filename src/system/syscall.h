#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#include "clib/stdint.h"
#include "system/proc.h"
#include "system/file_system.h"

typedef enum {
    SYS_OPEN,
    SYS_READ
} sys_call_t;

void syscall_handler(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

int sys_open(char *path, uint32_t flags);
// TODO replace path with file descriptor integer
int sys_read(char *path, void *buf, uint32_t count);

#endif // INCLUDE_SYSCALL_H