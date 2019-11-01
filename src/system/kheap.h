#ifndef INCLUDE_KHEAP_H
#define INCLUDE_KHEAP_H

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#include "print.h"

#define KHEAP_SIZE 4096

struct kernel_heap_segment_meta {
    size_t size;
    bool free;
    struct kernel_heap_segment_meta *next; 
};
typedef struct kernel_heap_segment_meta kernel_heap_segment_meta_t;

void kheap_init(void);
void *kmalloc(unsigned int bytes);
void kfree(void *pointer);
#ifdef DEBUG
void print_kmem_blocks(void);
#endif

#endif