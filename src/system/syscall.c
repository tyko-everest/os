#include "syscall.h"

ssize_t sys_read(const char *path, void *buf, size_t count, size_t from) {
    return fat32_readfile(path, from, count, buf);
}
