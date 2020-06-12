#include "page_frame.h"

static mem_segment_t *first_free_segment = NULL;

static void *temp_page_table = &temp_page_table_label;

uint32_t get_phys_addr(void* virt_addr) {

    uint32_t pd_index = (uint32_t) virt_addr >> 22;
    // pd is mapped to itself in the last virtual page
    uint32_t *pd = (uint32_t *) 0xFFFFF000;
    // check to see if pde doesn't exist
    if (pd[pd_index] == 0) {
        return NULL;
    }
    // check if this is a 4 MiB pde
    if (pd[pd_index] & 0x80) {
        return (pd[pd_index] & ~0x3FFFF) | ((uint32_t) virt_addr & 0x3FFFF);
    }

    uint32_t pt_index = ((uint32_t) virt_addr >> 12) & 0x3FF;
    uint32_t *pt = (uint32_t *) (0xFFC00000 + pd_index * PAGE_SIZE);
    // check to see if pte doesn't exist
    if (pt[pt_index] == 0) {
        return NULL;
    }
    return (pt[pt_index] & ~0xFFF) | ((uint32_t) virt_addr & 0xFFF);
}

/**
void init_free_memory(multiboot_info_t *mbt) {

    // allocate space for a temporary first free segment struct
    // needed to get a linked list going for the free segment list
    mem_segment_t *temp_first_seg = kmalloc(sizeof(mem_segment_t));
    if (temp_first_seg == NULL) {
        print("ERROR: cannot allocate memory\n", IO_OUTPUT_SERIAL);
        return;
    }

    mem_segment_t *curr_seg = temp_first_seg;
    
    // need offsets because we are in the higher half
    memory_map_t* mmap = (memory_map_t*) (mbt->mmap_addr + 0xC0000000);
    while((uint32_t) mmap < mbt->mmap_addr + mbt->mmap_length + 0xC0000000) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            // make the next new segment
            mem_segment_t *next_seg = kmalloc(sizeof(mem_segment_t));
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
            mem_segment_t *new_seg = kmalloc(sizeof(mem_segment_t));
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

    // now unmark any memory below 1 MB as free
    // for now we are going to be using the framebuffer and other mm i/o
    // TODO only case 2 has been tested (likely only useful one)
    curr_seg = first_free_segment;
    while (curr_seg != NULL && curr_seg->addr < 0x100000) {
        // no free segment below 1 MB
        if (curr_seg->addr >= 0x100000) {

        // free segment entirely below 1 MB
        } else if (curr_seg->addr + curr_seg->size - 1 < 0x100000) {
            curr_seg->next->prev = NULL;
            first_free_segment = curr_seg->next;
        // free segment goes across the 1 MB boundary
        } else {
            uint32_t loss = 0x100000 - curr_seg->next->addr;
            curr_seg->next->addr += loss;
            curr_seg->next->size -= loss;
        }
        curr_seg = curr_seg->next;
    }

}
**/

// TODO
// this a temporary version that manually sets it to what will work in bochs
// eventually it will detect memory properly
// need to ensure the segments start on 4 KiB boundary and are a multiple
// of that long 
void init_free_memory(multiboot_info_t *mbt) {
    first_free_segment = kmalloc(sizeof(mem_segment_t));
    if (first_free_segment == NULL) {
        print("ERROR: cannot allocate memory\n", IO_OUTPUT_SERIAL);
        return;
    }
    first_free_segment->prev = 0;
    first_free_segment->next = 0;
    // 1 MiB for GRUB / bios, then 4 MiB for the kernel page
    first_free_segment->addr = 0x100000 * 5;
    // rest of 32 MiB set in Bochs, minus bios rom at end
    first_free_segment->size = 0x100000 * (32 - 6);
}

// returns the physical address of an available page
// this page is marked as taken after it returns successfully
// so it is important to use it
uint32_t get_free_page(void) {
    // check if we have any free memory left
    uint32_t ret_addr = 0;
    if (first_free_segment != NULL) {
        // save this address of the start of the free page
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
        print("ERROR: no free pages", IO_OUTPUT_SERIAL);
        while(1);
    }
    return ret_addr;
}

// generates a new page table in a new page which can map the given virt addr
// helper function of allocate as it does not invalidate the tlb
// cannot be used alone
// TODO add safety check to flags
void generate_page_table(void *virt_addr, uint32_t flags) {
    uint32_t pd_index = (uint32_t) virt_addr >> 22;
    // pd is mapped to itself in the last virtual page
    uint32_t *pd = (uint32_t *) 0xFFFFF000;

    // check if there is already a pde here
    if (pd[pd_index] == 0) {
        // try to get a new page to store this page table
        uint32_t phys_addr = (uint32_t) get_free_page();
        if (phys_addr == NULL) {
            print("ERROR: failed to get mem for new page", IO_OUTPUT_SERIAL);
            while(1);
        } else {
            // setup the pde in the pd to point to this pt
            // set as read/write and present
            pd[pd_index] = (phys_addr & ~0xFFF) | flags;
        }
    } else {
        print("ERROR: pde is not empty", IO_OUTPUT_SERIAL);
        while(1);
    }
}

// allocates a physical page to the virtual page that corresponds with
// the given virtual address
// return the physical address of the newly assigned physical address
uint32_t allocate_page(void *virt_addr, uint32_t flags) {
    // tries to get a physical page to use
    uint32_t phys_addr = get_free_page();
    if (phys_addr == NULL) {
        print("ERROR: no mem for free page", IO_OUTPUT_SERIAL);
        while(1);
    }
    
    uint32_t pd_index = (uint32_t) virt_addr >> 22;
    // pd is mapped to itself in the last virtual page
    uint32_t *pd = (uint32_t *) 0xFFFFF000;

    uint32_t pt_index = ((uint32_t) virt_addr >> 12) & 0x3FF;
    uint32_t *pt = (uint32_t *) (0xFFC00000 + pd_index * PAGE_SIZE);

    // check if this is a 4 MiB pde, i.e. no page table below it
    if (pd[pd_index] & 0x80) {
        print("ERROR: pde already contains 4 MiB page", IO_OUTPUT_SERIAL);
        while(1);
    // see if there is no pde that covers this part in memory
    // if so we need to generate one
    } else if (pd[pd_index] == 0) {
        // make a new page table and set up its entry in the pd
        // set up with most permissive flags, as less permissive ones
        // will override any options below
        generate_page_table(virt_addr, USER_ACCESS | READ_WRITE | PRESENT);
    }
    // now we know a pde exists for this segment of virt memory
    // and it points to reserved space for a pt

    // check if there is no existing pte at this location
    if (pt[pt_index] == 0) {
        // set up according to desired flags
        pt[pt_index] = (phys_addr & ~0xFFF) | flags;
    } else {
        print("ERROR: page of virt_addr was not free", IO_OUTPUT_SERIAL);
    }

    // now invalidate the tlb for this page
    invalidate_tlb(virt_addr);

    return phys_addr;
}

#ifdef DEBUG
#include "stdio.h"

void print_free_memory(void) {
    mem_segment_t *curr_seg = first_free_segment;
    while (curr_seg != NULL) {
        printf("%X", curr_seg->addr);
        printf("%X", curr_seg->addr + curr_seg->size);
        printf("\n");
        curr_seg = curr_seg->next;
    }
}
#endif
