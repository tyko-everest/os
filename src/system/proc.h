#ifndef INCLUDE_PROC_H
#define INCLUDE_PROC_H

// has the definition for the register state struct
#include "clib/stdint.h"
#include "system/pm.h"
#include "system/mmu.h"
#include "system/vfs.h"
#include "utils/elf.h"

// used to store where a program is stored in phys memory
// and where these segments need to be loaded into virt memory
struct proc_mem_seg {
    uintptr_t phys_addr;
    void *virt_addr;
    size_t size;
    uint64_t attribs;
    struct proc_mem_seg *next;
};
typedef struct proc_mem_seg proc_mem_seg_t;

typedef struct {
    uint64_t x[32];
    struct {uint64_t low; uint64_t high;} v[32];
} general_regs_t;

typedef struct {
    uint64_t sp;
} system_regs_t;

typedef struct {
    size_t pid;
    proc_mem_seg_t *mem;
    general_regs_t gp_regs;
    system_regs_t sys_regs;
    int errno;

} process_t;

int add_fd_entry(const char *path);

void proc_load(const char *path, process_t *proc);
void proc_start(const process_t *proc);

#endif