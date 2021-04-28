#ifndef INCLUDE_PROC_H
#define INCLUDE_PROC_H

// has the definition for the register state struct
#include "system/interrupts.h"
#include "system/page_frame.h"
#include "system/vfs.h"
#include "utils/elf.h"

// used to store where a program is stored in phys memory
// and where these segments need to be loaded into virt memory
struct proc_mem_seg {
    uint32_t phys_addr, virt_addr, size;
    struct proc_mem_seg *next;
};
typedef struct proc_mem_seg proc_mem_seg_t;

typedef struct {
    uint32_t pid;
    proc_mem_seg_t *mem;
    stack_state_t registers;
    // inode of the program's current working directory
    uint32_t cwd;
    file_desc_t *fds;
    int errno;

} process_t;

int add_fd_entry(const char *path);

void proc_load(const char *path, process_t *proc);
void proc_start(const process_t *proc);

#endif