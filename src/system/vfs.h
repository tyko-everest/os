#ifndef INCLUDE_VFS
#define INCLUDE_VFS

#include "clib/stdlib.h"
#include "system/ext2.h"

typedef ext2_inode_t inode_t;

struct file_desc {
    uint32_t fd;
    uint32_t inode;
    uint32_t flags;
    struct file_desc *next;
};
typedef struct file_desc file_desc_t;

#define FD_PERM_READ    (1 << 0)
#define FD_PERM_WRITE   (1 << 1)

// helper functions
void fd_to_path(int fd, char);
uint32_t path_to_inode(const char *path, inode_t *ret_inode);

// syscall functions
int open(const char *pathname, int flags);
int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int lseek(int fd, int offset, int whence);

#endif
