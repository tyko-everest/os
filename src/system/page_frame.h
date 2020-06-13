#ifndef INCLUDE_PAGE_FRAME_H
#define INCLUDE_PAGE_FRAME_H

#include "clib/stddef.h"
#include "clib/stdint.h"
#include "clib/stdbool.h"
#include "utils/print.h"
#include "system/kheap.h"
#include "system/multiboot.h"

#define PAGE_SIZE 4096

#define KERNEL_ACCESS   (0 << 2)
#define USER_ACCESS     (1 << 2)
#define READ_ONLY       (0 << 1)
#define READ_WRITE      (1 << 1)
#define NOT_PRESENT     (0 << 0)
#define PRESENT         (1 << 0)

// notes these are NOT functions, they are labels exported by the linker
// the addresses of the these are the start and end of the kernel
void kernel_physical_start(void);
void kernel_physical_end(void);
// this is used for temp storage of new page tables
void temp_page_table_label(void);

struct mem_segment {
    uint32_t addr, size;
    struct mem_segment *prev, *next;
};
typedef struct mem_segment mem_segment_t;

void invalidate_tlb(void* virt_addr);
uint32_t get_phys_addr(void* virt_addr);
void init_free_memory(multiboot_info_t* mbt);
uint32_t get_free_page(void);
void generate_page_table(void *virt_addr, uint32_t flags);
uint32_t allocate_page(void *virt_addr, uint32_t phys_addr, uint32_t flags);


#ifdef DEBUG
void print_free_memory(void);
#endif

#endif
