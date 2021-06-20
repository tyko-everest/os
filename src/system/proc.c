#include "proc.h"

static process_t procs[16];
static size_t curr_pid = 0;
static size_t highest_pid = 0;

/**
 * TODO
 * 
 * - rethink how the functions are layed out
 * - instead of using proc_load_new or proc_load_existing maybe have one just load the struct 
 */

size_t get_curr_pid() {
    return curr_pid;
}

size_t get_parent(size_t pid) {
    return procs[pid].parent;
}

void get_gp_regs(size_t pid, general_regs_t *gp_regs) {
    memcpy(gp_regs, &procs[pid].gp_regs, sizeof(general_regs_t));
}

void save_regs(general_regs_t *gp_regs, system_regs_t *sys_regs) {
    memcpy(&procs[curr_pid].gp_regs, gp_regs, sizeof(general_regs_t));
    memcpy(&procs[curr_pid].sys_regs, sys_regs, sizeof(system_regs_t));
}

// TODO this needs to cleanup the pages used for the translation table as well
void proc_delete(size_t pid) {
    process_t proc = procs[pid];
    proc_mem_seg_t *seg = proc.mem;
    while (seg != NULL) {
        for (uintptr_t addr = seg->phys_addr; addr < seg->phys_addr + seg->size; addr += PAGE_SIZE) {
            pm_free_page(addr);
        }
        proc_mem_seg_t *tmp_seg = seg;
        seg = seg->next;
        kfree(tmp_seg);
    }
    seg = proc.stack;
    while (seg != NULL) {
        for (uintptr_t addr = seg->phys_addr; addr < seg->phys_addr + seg->size; addr += PAGE_SIZE) {
            pm_free_page(addr);
        }
        proc_mem_seg_t *tmp_seg = seg;
        seg = seg->next;
        kfree(tmp_seg);
    }
    vm_delete_ttb(proc.sys_regs.ttbr0_el1);
}

/**
 * Given the path to a program will load its contents into memory, save the mapping,
 * and setup things like the system registers to initial values for a new process
 * i.e. stack point at 1GB, elr_el1 at the program's entry, etc.
 */
int proc_new(const char *path) {
    elf64_header_t elf_header;
    elf64_prog_header_t prog_header[2];
    int a = parse_elf(path, &elf_header, prog_header);

    // see if this elf file is for the correct platform
    if (elf_header.e_ident[EI_CLASS] != ELFCLASS64 ||
        elf_header.e_ident[EI_DATA] != ELFDATA2LSB ||
        elf_header.e_ident[EI_VERSION] != EV_CURRENT ||
        elf_header.e_type != ET_EXEC ||
        elf_header.e_machine != EM_AARCH64 ||
        elf_header.e_version != EV_CURRENT) {
        
        return -1;
    }

    highest_pid++;
    size_t pid = highest_pid;
    procs[pid].pid = pid;
    procs[pid].parent = curr_pid;

    procs[pid].errno = 0;

    procs[pid].sys_regs.elr_el1 = elf_header.e_entry;
    procs[pid].sys_regs.sp_el0 = VM_USERSPACE_SIZE;
    procs[pid].sys_regs.spsr_el1 = 0;
    procs[pid].sys_regs.ttbr0_el1 = vm_new_ttb();

    // this program's translation table is used for loading into memory
    // so it must be set before any coping from disk is done
    WRITE_SYS_REG(ttbr0_el1, procs[pid].sys_regs.ttbr0_el1);
    asm("tlbi vmalle1");
    asm("dsb sy");
    asm("isb");

    // start with dummy first segment so other ones can be added onto it
    proc_mem_seg_t *dummy_seg = kmalloc(sizeof(proc_mem_seg_t));
    dummy_seg->next = NULL;
    proc_mem_seg_t *cur_seg = dummy_seg;
    memset(cur_seg, 0, sizeof(proc_mem_seg_t));

    for (int ph = 0; ph < elf_header.e_phnum; ph++) {
        // check if this needs to be loaded, and if it will be loaded
        // below 1 GB
        if (prog_header[ph].p_type == EPT_LOAD &&
            prog_header[ph].p_vaddr + prog_header[ph].p_memsz <= VM_USERSPACE_SIZE) {

            for (size_t page = 0; page < CEIL(prog_header[ph].p_memsz, VM_PAGE_SIZE); page++) {
            
                proc_mem_seg_t *next_seg = kmalloc(sizeof(proc_mem_seg_t));
                if (next_seg == NULL) {
                    // error
                    while(1);
                }

                next_seg->next = NULL;
                next_seg->virt_addr = (void *) prog_header[ph].p_vaddr + page * VM_PAGE_SIZE;
                next_seg->size = VM_PAGE_SIZE;

                next_seg->phys_addr = pm_get_page();
                if (next_seg->phys_addr == 0) {
                    while(1);
                }

                next_seg->attribs = VM_ATTR_INDEX(0) | VM_ACCESS_FLAG | VM_SHARE_INNER;
                if ((prog_header[ph].p_flags & EPF_X) == 0) {
                    next_seg->attribs |= VM_EL0_EXEC_DISABLE;
                }
                if (prog_header[ph].p_flags & EPF_R) {
                    if (prog_header[ph].p_flags & EPF_W) {
                        next_seg->attribs |= VM_USER_RW;
                        next_seg->attribs |= VM_EL1_EXEC_DISABLE;
                    } else {
                        next_seg->attribs |= VM_USER_RO;
                    }
                } else {
                    next_seg->attribs |= VM_KERNEL_RO;
                }

                // transfer contents of elf file into memory

                // temporarily map this section with kernel RW permissions so it can copy the memory
                vm_map_page(next_seg->virt_addr, next_seg->phys_addr, VM_ATTR_INDEX(0) | VM_ACCESS_FLAG | VM_SHARE_INNER | VM_KERNEL_RW, procs[pid].sys_regs.ttbr0_el1);
                // now copy the contents of this program data to memory
                readfile(path, prog_header[ph].p_offset + page * VM_PAGE_SIZE, prog_header[ph].p_filesz % VM_PAGE_SIZE, next_seg->virt_addr);
                // set the correct VM attributes for the page
                vm_map_page(next_seg->virt_addr, next_seg->phys_addr, next_seg->attribs, procs[pid].sys_regs.ttbr0_el1);

                // check if we just created a continuous memory section
                // if so merge the two sections
                if (next_seg->attribs == cur_seg->attribs &&
                    next_seg->phys_addr == cur_seg->phys_addr + cur_seg->size &&
                    next_seg->virt_addr == cur_seg->virt_addr + cur_seg->size) {

                    cur_seg->size += VM_PAGE_SIZE;
                    kfree(next_seg);

                } else {
                    // setup cur_seg to point to newly setup next_seg
                    cur_seg->next = next_seg;
                    // change cur_seg to next_seg for next iteration
                    cur_seg = next_seg;
                }
            }
        }
    }
    procs[pid].mem = dummy_seg->next;
    kfree(dummy_seg);

    procs[pid].stack = kmalloc(sizeof(proc_mem_seg_t));
    procs[pid].stack->attribs = VM_ACCESS_FLAG | VM_SHARE_INNER | VM_USER_RW | VM_EL0_EXEC_DISABLE;
    procs[pid].stack->virt_addr = (void *) (VM_USERSPACE_SIZE - VM_PAGE_SIZE);
    procs[pid].stack->phys_addr = pm_get_page();
    procs[pid].stack->size = VM_PAGE_SIZE;
    procs[pid].stack->next = NULL;
    vm_map_page(procs[pid].stack->virt_addr, procs[pid].stack->phys_addr, procs[pid].stack->attribs, procs[pid].sys_regs.ttbr0_el1);

    return pid;
}

/**
 * Setup the memory mapping for the given process
 */
int proc_load(size_t pid) {

    proc_mem_seg_t *cur_seg = procs[pid].mem;
    while (cur_seg != NULL) {
        for (size_t i = 0; i < CEIL(cur_seg->size, VM_PAGE_SIZE); i++) {
            vm_map_page((void*) ((uintptr_t) cur_seg->virt_addr + VM_PAGE_SIZE * i), cur_seg->phys_addr + VM_PAGE_SIZE * i, cur_seg->attribs, procs[pid].sys_regs.ttbr0_el1);
        }
        cur_seg = cur_seg->next;
    }

    cur_seg = procs[pid].stack;
    while (cur_seg != NULL) {
        for (size_t i = 0; i < CEIL(cur_seg->size, VM_PAGE_SIZE); i++) {
            vm_map_page((void*) ((uintptr_t) cur_seg->virt_addr + VM_PAGE_SIZE * i), cur_seg->phys_addr + VM_PAGE_SIZE * i, cur_seg->attribs, procs[pid].sys_regs.ttbr0_el1);
        }
        cur_seg = cur_seg->next;
    }

    return 0;
}

/**
 * Sets the system registers to those of a given process and flushes the tlb.
 * After calling this function eret will switch execution to the given process
 */
void proc_switch(size_t pid) {
    WRITE_SYS_REG(spsr_el1, procs[pid].sys_regs.spsr_el1);
    WRITE_SYS_REG(sp_el0, procs[pid].sys_regs.sp_el0);
    WRITE_SYS_REG(elr_el1, procs[pid].sys_regs.elr_el1);
    WRITE_SYS_REG(ttbr0_el1, procs[pid].sys_regs.ttbr0_el1);
    curr_pid = pid;
    asm("tlbi vmalle1");
    asm("dsb sy");
    asm("isb");
}
