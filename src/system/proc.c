#include "system/proc.h"

static process_t curr_proc;

void proc_load(const char *path, process_t *proc) {
    elf32_header_t elf_header;
    elf32_prog_header_t prog_header[2];
    int a = parse_elf(path, &elf_header, prog_header);

    // start with dummy first segment so other ones can be added onto it
    proc_mem_seg_t *dummy_seg = kmalloc(sizeof(proc_mem_seg_t));
    dummy_seg->next = NULL;
    proc_mem_seg_t *cur_seg = dummy_seg;

    for (int i = 0; i < 2; i++) {
        // check if this needs to be loaded, and if it will be loaded
        // below 3 GiB
        if (prog_header[i].p_type == EPT_LOAD &&
            prog_header[i].p_vaddr + prog_header[i].p_memsz <= 0xC0000000) {
            
            proc_mem_seg_t *next_seg = kmalloc(sizeof(proc_mem_seg_t));
            if (next_seg == NULL) {
                // error
                while(1);
            }

            next_seg->next = NULL;
            next_seg->virt_addr = prog_header[i].p_vaddr;
            next_seg->size = prog_header[i].p_memsz;

            // for now only handle allocating one page
            if (next_seg->size > 0x1000) {
                while(1);
            }

            next_seg->phys_addr = get_free_page();
            if (next_seg->phys_addr == 0) {
                while(1);
            }

            // transfer contents of elf file into memory
            // TODO, get file size from file system and make buffer from this
            // this whole section is VERY hard coded
            uint8_t buf[1024*5];
            fs_readfile(path, 0, 5, buf);
            // setup the page_buf so it maps to the desired phys memory
            uint32_t flags = USER_ACCESS | READ_WRITE | PRESENT;
            // temporarily using the first page to copy this data
            allocate_page(0, next_seg->phys_addr, flags);
            // now copy the contents of this program data to memory
            memcpy(0, buf + prog_header[i].p_offset, prog_header[i].p_filesz);

            // setup cur_seg to point to newly setup next_seg
            cur_seg->next = next_seg;
            // change cur_seg to next_seg for next iteration
            cur_seg = next_seg;

        }
    }
    proc->mem = dummy_seg->next;
    kfree(dummy_seg);

    // TODO, unique pids
    proc->pid = 1;

    // user data
    proc->registers.ds = 0x20;
    proc->registers.esp = 0xC0000000 - 4;
    proc->registers.ebp = 0xC0000000 - 4;
    // rest of registers don't need to be initialized to anything in particular
    
}

void proc_start(const process_t *proc) {
    // load program and data memory
    proc_mem_seg_t *cur_seg = proc->mem;
    while (cur_seg != NULL) {
        uint32_t flags = USER_ACCESS | READ_WRITE | PRESENT;
        // TODO this will need to change to handle segments > 1 page big
        cur_seg->phys_addr =
            allocate_page(cur_seg->virt_addr, cur_seg->phys_addr, flags);

        cur_seg = cur_seg->next;
    }

    // load in stack
    uint32_t phys_stack = get_free_page();
    if (phys_stack == 0)
        // error
        while(1);
    // load the stack TODO, decide where this is best put
    uint32_t flags = USER_ACCESS | READ_WRITE | PRESENT;
    // temporarily using the first page to copy this data
    allocate_page(0xC0000000 - 0x1000, phys_stack, flags);

    // TODO, this will set the registers back to their previous values

    cpu_state_t cpu_state;
    cpu_state.cs = 0x18 | 0x03;
    cpu_state.eflags = 0;
    cpu_state.eip = 0;
    cpu_state.error_code = 0;
    cpu_state.esp = 0xC0000000 - 4;
    cpu_state.ss = 0x20 | 0x03;

    enter_user_mode();

}

