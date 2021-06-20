#ifndef INCLUDE_MMU_H
#define INCLUDE_MMU_H

/**
 * The Memory Management Unit
 * 
 * Plan:
 * - using short-descriptor translation table format
 * - kernel will have 512 GB VM space (only going to use 2 GB of that though)
 * - 1 GB flat map at 0xFFFFFF8000000000 for normal memory
 * - 1 GB flat map at next GB, for peripheral memory (only use last 16 MB)
 * 
 * - user space will have 1 GB VM only
 * - this means translation starts at level 2, TTBR0.SZ = 34
 * 
 */

#include "arch/arm.h"
#include "clib/stddef.h"
#include "clib/stdint.h"
#include "clib/stdbool.h"
#include "clib/string.h"
#include "system/kheap.h"
#include "system/pm.h"
#include "utils/printf.h"

#define VM_USERSPACE_SIZE (1 << 30)

#define VM_LOG_PAGE_SIZE 12
#define VM_PAGE_SIZE (1 << VM_LOG_PAGE_SIZE)

#define VM_BLOCK    0b01
#define VM_TABLE    0b11
#define VM_PAGE     0b11

#define VM_ATTR_INDEX(x)    ((x) << 3)
#define VM_KERNEL_RW        (0b00UL << 6)
#define VM_KERNEL_RO        (0b10UL << 6)
#define VM_USER_RW          (0b01UL << 6)
#define VM_USER_RO          (0b11UL << 6)
#define VM_SHARE_NONE       (0b00UL << 8)
#define VM_SHARE_OUTER      (0b10UL << 8)
#define VM_SHARE_INNER      (0b11UL << 8)
#define VM_ACCESS_FLAG      (0b1UL << 10)
#define VM_EL1_EXEC_DISABLE (0b1UL << 53)
#define VM_EL0_EXEC_DISABLE (0b1UL << 54)

uint64_t read_sctlr_el1();

void vm_init();

// allocate a new page for the first level of a translation table
// and clear the memory so all entries are invalid to start
uintptr_t vm_new_ttb();

// free all pages associated with a translation table mapping
void vm_delete_ttb();

/**
 * allocates a page of PM at a desired VM address in userspace
 * i.e. the range is 0 -> 1 GB
 * 
 * @param virt_addr the virtual address of the start of the page
 * @param phys_addr the physical address of the start of a page
 * @param attribs the desired properties of the memory
 * @param ttb the physical address of the translation table base
 * @return 0 on success, negative on error
 */
int vm_map_page(void *virt_addr, uintptr_t phys_addr, uint64_t attribs, uintptr_t ttb);




#endif
