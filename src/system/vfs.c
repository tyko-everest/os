#include "vfs.h"

uint32_t path_to_inode(const char *path, inode_t *ret_inode) {
    fs_path_to_inode(path, ret_inode);
}

int open(const char *pathname, int flags) {
    
}
