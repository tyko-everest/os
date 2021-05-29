#include "pm.h"

// linker symbols
extern char __free_mem_start[];
extern char __virtual_start[];

static mem_segment_t *first_free_segment = NULL;

// hardcoded initialization of what memory is free on the pi 
void pm_init() {
    first_free_segment = kmalloc(sizeof(mem_segment_t));
    if (first_free_segment == NULL) {
        printf("ERROR: cannot allocate memory\n");
        return;
    }
    first_free_segment->prev = NULL;
    first_free_segment->next = NULL;
    // free space symbol starts after the end of the kernel as per the linker
    first_free_segment->addr = (uintptr_t) __free_mem_start - (uintptr_t) __virtual_start;
    // free space size is the 1GB of memory, minus where the kernel ends,
    // minus 16 MB for peripherals, minus 16 MB for gpu
    first_free_segment->size = (1 << 30) - first_free_segment->addr - (1 << 24) * 2;
}

// returns the physical address of an available page
// this page is marked as taken after it returns successfully
// so it is important to use it
uintptr_t pm_get_page() {
    // check if we have any free memory left
    uintptr_t ret_addr = 0;
    if (first_free_segment != NULL) {
        // save the address of the start of the free page
        ret_addr = first_free_segment->addr;

        first_free_segment->addr += PAGE_SIZE;
        first_free_segment->size -= PAGE_SIZE;

        // check if we have killed off an entire free segment
        if (first_free_segment->size == 0) {
            // save the current list pointer, as it will change below
            mem_segment_t *temp = first_free_segment;
            // removing first segment, so cleanup the pointer of next segment
            if (first_free_segment->next != NULL) {
                first_free_segment = first_free_segment->next;
                first_free_segment->prev = NULL;
            // last node, set list pointer to NULL
            } else {
                first_free_segment = NULL;
            }
            // free the struct of the removed segment
            kfree(temp);            
        }
    // no memory left, throw an error
    } else {
        printf("ERROR: no free pages\n");
        while(1);
    }
    return ret_addr;
}

void print_free_memory() {
    mem_segment_t *curr_seg = first_free_segment;
    while (curr_seg != NULL) {
        printf("segment start: %lX", curr_seg->addr);
        printf("\tsegment end: %lX\n", curr_seg->addr + curr_seg->size);
        curr_seg = curr_seg->next;
    }
}
