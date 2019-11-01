#ifndef INCLUDE_PAGE_FRAME_H
#define INCLUDE_PAGE_FRAME_H

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#include "print.h"
#include "kheap.h"
#include "multiboot.h"

// notes these are NOT functions, they are label exported by the linker
// the addresses of the these are the start and end of the kernel
void kernel_physical_start(void);
void kernel_physical_end(void);

struct free_mem_segment {
    uint32_t addr, size;
    struct free_mem_segment *prev, *next;
};
typedef struct free_mem_segment free_mem_segment_t;

// below is for the allocation of page frames
void init_free_memory(multiboot_info_t* mbt);

#ifdef DEBUG
void print_free_memory(void);
#endif

#endif
