#ifndef INCLUDE_PROC_H
#define INCLUDE_PROC_H

// has the definition for the register state struct
#include "arch/arm.h"
#include "clib/stdint.h"
#include "system/pm.h"
#include "system/mmu.h"
#include "system/vfs.h"
#include "utils/elf.h"

// stores the mapping between physical and virtual memory for the process
// also stores the attributes it needs in the mmu
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
    uint64_t sp_el0;
    uint64_t elr_el1;
    uint64_t spsr_el1;
    uint64_t ttbr0_el1;
} system_regs_t;

typedef struct {
    size_t pid, parent;
    proc_mem_seg_t *mem;
    proc_mem_seg_t *stack;
    general_regs_t gp_regs;
    system_regs_t sys_regs;
    int errno;
} process_t;

size_t get_curr_pid();
size_t get_parent(size_t pid);
void get_gp_regs(size_t pid, general_regs_t *gp_regs);
void save_regs(general_regs_t *gp_rgs, system_regs_t *sys_regs);
void proc_delete(size_t pid);

int proc_new(const char *path);
int proc_load(size_t pid);
void proc_switch(size_t pid);

#endif