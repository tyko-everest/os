#include "vfs.h"

void vfs_init() {
    fat32_init();
}

int readfile(const char *path, uint32_t start, uint32_t num, uint8_t *buf) {
    return fat32_readfile(path, start, num, buf);
}

int writefile(const char *path, uint32_t start, uint32_t num, uint8_t *buf) {
    return fat32_writefile(path, start, num, buf);
}

int makefile(const char *path, bool is_dir) {
    return fat32_makefile(path, is_dir);
}

int deletefile(const char *path) {
    return deletefile(path);
}
