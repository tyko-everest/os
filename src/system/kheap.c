#include "kheap.h"

// used to get address of where the fixed kernel heap starts in memory
// this "function" is just a label to where the heap starts in assembly
void kernel_heap(void);
// TODO: have these constants share references with same asm constants
// TODO: not a fixed size heap for kernel
uint32_t kernel_heap_size = KHEAP_SIZE;
kernel_heap_segment_meta_t *kernel_heap_first_segment = NULL;

void kheap_init() {
    // setup the start of the heap as a segment meta for the whole space
    // starts as a single block of all space marked free
    ((kernel_heap_segment_meta_t*) kernel_heap)->size =
            kernel_heap_size - sizeof(kernel_heap_segment_meta_t);
    ((kernel_heap_segment_meta_t*) kernel_heap)->free = true;
    ((kernel_heap_segment_meta_t*) kernel_heap)->next = NULL;

    // setup linked list to start pointing to this segment
    kernel_heap_first_segment = (kernel_heap_segment_meta_t*) kernel_heap;
}

void *kmalloc(uint32_t bytes) {
    // handle the case where no memory is requested
    // just return NULL, this is a useless thing to ask for
    if (bytes == 0) {
        return NULL;
    }

    // round the size up to the nearest 4 bytes
    // this will ensure alignment of all 32 bit data types
    if (bytes % sizeof(uint32_t) != 0) {
        bytes = bytes / sizeof(uint32_t) * sizeof(uint32_t) + sizeof(uint32_t);
    }
    // iterate through all the segments and end when there is none left
    // and we have not found a suitable block
    kernel_heap_segment_meta_t *curr_seg = kernel_heap_first_segment;
    while (curr_seg != NULL) {

        // move on if either this block is used or if it is too small
        if (curr_seg->free == false || curr_seg->size < bytes
                + sizeof(kernel_heap_segment_meta_t)) {
            curr_seg = curr_seg->next;

        // this means we have found a free block that is big enough
        // we will just take the portion of this block we need
        } else {
            // total new size of the current block
            uint32_t total_new_size =
                    bytes + sizeof(kernel_heap_segment_meta_t);
            // first we setup the meta data for the new segment we are making
            // after the newly shortened current segment
            kernel_heap_segment_meta_t *new_block =
                    (kernel_heap_segment_meta_t*) ((uint32_t) curr_seg
                    + total_new_size);
            // new second block should be everything left over
            new_block->size = curr_seg->size - total_new_size;
            new_block->free = true;
            new_block->next = curr_seg->next;

            // now change the current block to reflect new reality
            curr_seg->size = bytes;
            curr_seg->free = false;
            curr_seg->next = new_block;

            // return the address of the start of the usable space
            // ie. meta data pointer plus its size
            // remember pointer addition adds its size in bytes!
            return curr_seg + 1;
        }
    }
    // if we made it all the way out here that means we never found
    // a suitable segment
    return NULL;
}

// TODO: add merging with previous blocks as well
void kfree(void *pointer) {
    if (pointer == NULL) {
        return;
    }
    kernel_heap_segment_meta_t *curr_seg = kernel_heap_first_segment;
    kernel_heap_segment_meta_t *prev_seg = NULL;
    while (curr_seg != NULL) {
        // curr_seg plus size is the address in bytes after meta data
        // + 1 with pointers adds the size of the pointer type in bytes
        // if these match we have found our block
        if ((uint8_t*) (curr_seg + 1) == (uint8_t*) pointer) {
            curr_seg->free = true;
            // see if we can merge with the next block
            if (curr_seg->next != NULL && curr_seg->next->free == true) {
                curr_seg->size += curr_seg->next->size
                        + sizeof(kernel_heap_segment_meta_t);
                curr_seg->next = curr_seg->next->next;
            }
            // see if we can merge with previous block
            if (prev_seg != NULL && prev_seg->free == true) {
                prev_seg->size += curr_seg->size +
                        sizeof(kernel_heap_segment_meta_t);
                prev_seg->next = curr_seg->next;
            }
            return;
        } else {
            prev_seg = curr_seg;
            curr_seg = curr_seg->next;
        }
    }
}

#ifdef DEBUG
#include "stdio.h"
void print_kmem_blocks(void) {
    kernel_heap_segment_meta_t *curr_seg = kernel_heap_first_segment;
    while (curr_seg != NULL) {
        printf("%d", curr_seg->size);
        printf("%d", curr_seg->free);
        printf("\n");
        curr_seg = curr_seg->next;
    }
}
#endif
