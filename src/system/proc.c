#include "proc.h"

static process_t procs[16];
static process_t curr_pid;

int proc_load(const char *path, process_t *proc) {
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

    proc->errno = 0;

    // TODO, unique pids
    proc->pid = 1;

    proc->sys_regs.elr_el1 = elf_header.e_entry;
    proc->sys_regs.sp_el0 = (1 << 30);
    proc->sys_regs.spsr_el1 = 0;
    proc->sys_regs.ttbr0_el1 = vm_new_ttb();

    // this program's translation table is used for loading into memory
    // so it must be set before any coping from disk is done
    WRITE_SYS_REG(ttbr0_el1, proc->sys_regs.ttbr0_el1);
    asm("tlbi vmalle1");
    asm("dsb sy");
    asm("isb");

    // start with dummy first segment so other ones can be added onto it
    proc_mem_seg_t *dummy_seg = kmalloc(sizeof(proc_mem_seg_t));
    dummy_seg->next = NULL;
    proc_mem_seg_t *cur_seg = dummy_seg;

    for (int i = 0; i < elf_header.e_phnum; i++) {
        // check if this needs to be loaded, and if it will be loaded
        // below 1 GB
        if (prog_header[i].p_type == EPT_LOAD &&
            prog_header[i].p_vaddr + prog_header[i].p_memsz <= (1 << 30)) {
            
            proc_mem_seg_t *next_seg = kmalloc(sizeof(proc_mem_seg_t));
            if (next_seg == NULL) {
                // error
                while(1);
            }

            next_seg->next = NULL;
            next_seg->virt_addr = (void *) prog_header[i].p_vaddr;
            next_seg->size = prog_header[i].p_memsz;

            // for now only handle allocating one page
            if (next_seg->size > 0x1000) {
                while(1);
            }

            next_seg->phys_addr = pm_get_page();
            if (next_seg->phys_addr == 0) {
                while(1);
            }

            next_seg->attribs = VM_ATTR_INDEX(0) | VM_ACCESS_FLAG | VM_SHARE_INNER;
            if ((prog_header[i].p_flags & EPF_X) == 0) {
                next_seg->attribs |= VM_EL0_EXEC_DISABLE;
            }
            if (prog_header[i].p_flags & EPF_R) {
                if (prog_header[i].p_flags & EPF_W) {
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
            vm_allocate_page(next_seg->virt_addr, next_seg->phys_addr, VM_ATTR_INDEX(0) | VM_ACCESS_FLAG | VM_SHARE_INNER | VM_KERNEL_RW, proc->sys_regs.ttbr0_el1);
            // now copy the contents of this program data to memory
            fat32_readfile(path, prog_header[i].p_offset, prog_header[i].p_filesz, next_seg->virt_addr);
            // set the correct VM attributes for the page
            vm_allocate_page(next_seg->virt_addr, next_seg->phys_addr, next_seg->attribs, proc->sys_regs.ttbr0_el1);
            // setup cur_seg to point to newly setup next_seg
            cur_seg->next = next_seg;
            // change cur_seg to next_seg for next iteration
            cur_seg = next_seg;

        }
    }
    proc->mem = dummy_seg->next;
    kfree(dummy_seg);

    return 0;
}

void proc_start(const process_t *proc) {

    // load program and data memory
    proc_mem_seg_t *cur_seg = proc->mem;
    while (cur_seg != NULL) {
        vm_allocate_page(cur_seg->virt_addr, cur_seg->phys_addr, cur_seg->attribs, proc->sys_regs.ttbr0_el1);
        cur_seg = cur_seg->next;
    }

    // load in stack
    uintptr_t phys_stack = pm_get_page();
    if (phys_stack == 0)
        while(1);
    // load the stack TODO, decide where this is best put
    uint64_t attribs = VM_ACCESS_FLAG | VM_SHARE_INNER | VM_USER_RW | VM_EL0_EXEC_DISABLE;

    vm_allocate_page((1 << 30) - 0x1000, phys_stack, attribs, proc->sys_regs.ttbr0_el1);

    WRITE_SYS_REG(spsr_el1, proc->sys_regs.spsr_el1);
    WRITE_SYS_REG(sp_el0, proc->sys_regs.sp_el0);
    WRITE_SYS_REG(elr_el1, proc->sys_regs.elr_el1);
    WRITE_SYS_REG(ttbr0_el1, proc->sys_regs.ttbr0_el1);
    asm("eret");

}
