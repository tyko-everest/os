#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#include "clib/stdint.h"
#include "system/proc.h"
#include "system/file_system.h"
#include "utils/print.h"

typedef enum {
    SYS_OPEN,
    SYS_READ,
    SYS_PRINT
} sys_call_t;

/***
 * General TODO
 * There is no security in these at all. 
 * Eventually will need to actually check if they have permission
 * to be reading from the memory locations they want to be,
 * or for the given file etc.
 */

void syscall_handler(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

int sys_open(char *path, uint32_t flags);
// TODO replace path with file descriptor integer
int sys_read(char *path, void *buf, uint32_t count);
// print a standard string to console, no formatting
int sys_print(char *msg);

#endif // INCLUDE_SYSCALL_H