#include "syscall.h"

void syscall_handler(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    switch (num) {
    case SYS_OPEN:
        // TODO will eax stay the same after the handler returns?
        sys_open((char *) arg1, arg2);
        break;
    case SYS_READ:
        sys_read((char *) arg1, (void *) arg2, arg3);
        break;
    case SYS_PRINT:
        sys_print((char *) arg1);
    default:
        break;
    }
}

int sys_open(char *path, uint32_t flags) {
    return -1;
}

// reads from start of file count bytes
// absolute paths only
int sys_read(char *path, void *buf, uint32_t count) {
    return fs_readfile(path, 0, count, buf);
}

int sys_print(char *msg) {
    print(msg, IO_OUTPUT_FB);
    return 0;
}