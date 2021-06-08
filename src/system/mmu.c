#include "mmu.h"

extern char __virtual_start[];

void vm_init() {
    uint64_t tcr;
    READ_SYS_REG(tcr_el1, tcr);
    tcr &= ~0b111111;
    tcr |= 34;
    WRITE_SYS_REG(tcr_el1, tcr);
    
    asm("tlbi vmalle1");
    asm("dsb sy");
    asm("isb");
}

uintptr_t vm_new_ttb() {
    uintptr_t ttb = pm_get_page();
    memset(ttb + (size_t) __virtual_start, 0, VM_PAGE_SIZE);
    return ttb;
}

int vm_allocate_page(void *virt_addr, uintptr_t phys_addr, uint64_t attribs, uintptr_t ttb) {

    // since kernel is flat mapped, converting PM to kernel VM addresses
    // is simply addition
    size_t phys_to_virt_offset = (size_t) __virtual_start;
    
    size_t lv2_offset = (((uintptr_t) virt_addr) >> (12 + 9)) & 0x1FF;
    size_t lv3_offset = (((uintptr_t) virt_addr) >> 12) & 0x1FF;

    uint64_t *lv2_table = (uint64_t *) ((ttb & 0xFFFFFFFFF000) + phys_to_virt_offset);
    if ((lv2_table[lv2_offset] & 0b1) == 0) {
        // TODO add check
        uintptr_t new_lv3_table = pm_get_page();
        memset((void *) (new_lv3_table + phys_to_virt_offset), 0, VM_PAGE_SIZE);
        lv2_table[lv2_offset] = new_lv3_table | VM_TABLE;
    }

    uint64_t *lv3_table = (uint64_t *) ((lv2_table[lv2_offset] & 0xFFFFFFFFF000) + phys_to_virt_offset);
    lv3_table[lv3_offset] = phys_addr | attribs | VM_PAGE;
    
    size_t page_num = (size_t) virt_addr >> VM_LOG_PAGE_SIZE;
    asm (
        "tlbi vaae1, %[pn]"
        :
        : [pn] "r" (page_num)
    );
    asm("dsb sy");
    asm("isb");
    return 0;
}
