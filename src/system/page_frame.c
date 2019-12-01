#include "page_frame.h"

free_mem_segment_t *first_free_segment = NULL;

void init_free_memory(multiboot_info_t *mbt) {

    // allocate space for a temporary first free segment struct
    // needed to get a linked list going for the free segment list
    free_mem_segment_t *temp_first_seg = kmalloc(sizeof(free_mem_segment_t));
    if (temp_first_seg == NULL) {
        print("ERROR: cannot allocate memory\n", IO_OUTPUT_SERIAL);
        return;
    }

    free_mem_segment_t *curr_seg = temp_first_seg;
    
    // need offsets because we are in the higher half
    memory_map_t* mmap = (memory_map_t*) (mbt->mmap_addr + 0xC0000000);
    while((uint32_t) mmap < mbt->mmap_addr + mbt->mmap_length + 0xC0000000) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            // make the next new segment
            free_mem_segment_t *next_seg = kmalloc(sizeof(free_mem_segment_t));
            if (next_seg == NULL) {
                print("ERROR: cannot allocate memory\n", IO_OUTPUT_SERIAL);
                return;
            }
            next_seg->addr = mmap->base_addr_low;
            next_seg->size = mmap->length_low;
            next_seg->prev = curr_seg;
            next_seg->next = NULL;
            // ensure the curr segment points to this new one
            curr_seg->next = next_seg;
            // the next segment is now the current one
            curr_seg = next_seg;
        }
        // print_uint(mmap->base_addr_low);
        // print_uint(mmap->length_low);
        // print_uint(mmap->type);
        // print("\n", IO_OUTPUT_FB);
        mmap = (memory_map_t*) ((unsigned int)mmap
                + mmap->size + sizeof(mmap->size));
    }

    // check if curr_seg still points towards temp_first_seg
    // if so we did not find any free memory and throw an error
    if (curr_seg == temp_first_seg) {
        print("ERROR: no free memory found\n", IO_OUTPUT_SERIAL);
        return;
    }
    // now we must remove the temporary first free mem struct from the start
    // of the list and set the pointer to the start of the list
    first_free_segment = temp_first_seg->next;
    first_free_segment->prev = NULL;
    kfree(temp_first_seg);

    // now we must go through and mark the kernel's memory as used
    uint32_t kernel_addr = (uint32_t) &kernel_physical_start;
    uint32_t kernel_size = (uint32_t) (&kernel_physical_end -
            &kernel_physical_start);

    // now to find which free segment this exists in
    // (almost certainly at the beginning of a segment, but we'll check)
    curr_seg = first_free_segment;
    // TODO: check cases 2 and 3, only 1 is tested working
    // ie, link it at not 1 MiB but a little further
    // and see if it will split it properly
    while (curr_seg != NULL) {

        // check to see if it exists at the start of this segment
        if (kernel_addr == curr_seg->addr) {
            curr_seg->addr += kernel_size;
            curr_seg->size -= kernel_size;
            break;

        // check if it finishes at the end of a segment
        } else if (kernel_addr + kernel_size ==
                   curr_seg->addr + curr_seg->size) {
            curr_seg->size -= kernel_size;
            break;

        // its in the middle, we need to make a new free segment
        } else if (kernel_addr < curr_seg->addr + curr_seg->size - 1) {
            // make a new segment to go after the current segment the kernel
            // is splitting, memory: curr_seg -> kernel -> new_seg
            free_mem_segment_t *new_seg = kmalloc(sizeof(free_mem_segment_t));
            if (new_seg == NULL) {
                print("ERROR: cannot allocate memory\n", IO_OUTPUT_SERIAL);
                return;
            }
            new_seg->prev = curr_seg;
            new_seg->next = curr_seg->next;
            // check to see if there was another segment after curr_seg
            if (curr_seg->next != NULL) {
                curr_seg->next->prev = new_seg;
            }
            curr_seg->next = new_seg;

            // resize to fill in the remainder after the kernel
            new_seg->size = (curr_seg->addr + curr_seg->size) -
                    (kernel_addr + kernel_size) + 1;

            // resize to take up the space up till the start of the kernel
            curr_seg->size = kernel_addr - curr_seg->addr;

        // not in this segment, go to the next one
        } else {
            curr_seg = curr_seg->next;
        }
    }
}

// returns the physical address of an available page
void* get_free_page(void) {
    // check if we have any free memory left
    if (first_free_segment != NULL) {
        first_free_segment->addr += PAGE_SIZE;
        first_free_segment->size -= PAGE_SIZE;

        // check if we have killed off an entire free segment
        if (first_free_segment->size == 0) {
            // save the current list pointer, as it will change below
            free_mem_segment_t *temp = first_free_segment;
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
        print("ERROR: no free pages", IO_OUTPUT_SERIAL);
        while(1);
    }
}

#ifdef DEBUG
void print_free_memory(void) {
    free_mem_segment_t *curr_seg = first_free_segment;
    while (curr_seg != NULL) {
        print_uint(curr_seg->addr);
        print_uint(curr_seg->size);
        print_nl();
        curr_seg = curr_seg->next;
    }
}
#endif
