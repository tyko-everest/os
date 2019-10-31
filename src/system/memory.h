#ifndef INCLUDE_MEMORY_H
#define INCLUDE_MEMORY_H

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#include "print.h"
#include "multiboot.h"

#define MAX_FREE_MEM_SEGMENTS 204

struct kernel_heap_segment_meta {
    size_t size;
    bool free;
    struct kernel_heap_segment_meta *next; 
};
typedef struct kernel_heap_segment_meta kernel_heap_segment_meta_t;

struct free_mem_segment {
    unsigned int addr_low, addr_high;
    struct free_mem_segment *prev, *next;
    // index of the mem_segment array these will be stored in
    // temp solution until the kernel heap is implemented
    unsigned int index;
};
typedef struct free_mem_segment free_mem_segment_t;

void kheap_init(void);

void *kmalloc(unsigned int bytes);

void kfree(void *pointer);

#ifdef DEBUG
void print_kmem_blocks(void);
#endif

void init_free_memory(multiboot_info_t* mbt);

#endif