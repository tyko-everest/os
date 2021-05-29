#ifndef INCLUDE_PM_H
#define INCLUDE_PM_H

#include "clib/stdbool.h"
#include "clib/stddef.h"
#include "clib/stdint.h"
#include "system/kheap.h"

#define PAGE_SIZE 4096

struct mem_segment {
    uintptr_t addr;
    size_t size;
    struct mem_segment *prev, *next;
};
typedef struct mem_segment mem_segment_t;

void pm_init();
uintptr_t pm_get_page();
void print_free_memory();

#endif
