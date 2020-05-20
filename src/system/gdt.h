#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

#include "print.h"
#include "stdint.h"

#define GDT_SIZE 6

typedef struct __attribute__((packed)){
    unsigned short size;
    unsigned int address;
} gdt_header_t;

typedef struct __attribute__((packed)){
    unsigned short limit0_15;
    unsigned short base0_15;
    unsigned char base16_23;
    unsigned char access_byte;
    unsigned char flags_limit16_19;
    unsigned char base24_31;
} gdt_entry_t;

typedef struct __attribute((packed)) {
    unsigned int LINK;
    unsigned int ESP0;
    unsigned int SS0;
    unsigned int RESERVED[22];
    unsigned int IOPB;
} tss_t;

/** load_gdt:
 * Loads the specified gdt in assembly
 * @param gdt The address of a gdt_info struct
 */
void load_gdt(gdt_header_t gdt_info);

/** load_tss:
 * Loads the specified tss
 * @param segment the segment selector in the gdt to use
 */
void load_tss(uint32_t segment);

/** load_segment_registers
 * Loads all the segment registers with the proper segment descriptors
 */
void load_segment_registers();

// adds a new entry to the gdt, automatically incrmenents to the next entry
// after each call, make sure to call once for NULL entry
// also flags assumes the Gr and Sz are in bits 7 and 6 already
void add_gdt_entry(uint32_t limit, uint32_t base, uint8_t access_byte,
                   uint8_t flags);

// setup a standard flat memory config for kernel and user
void setup_flat_memory(void);

// add the tss to the gdt and load the tss
void setup_tss(uint32_t kernel_stack_base);

#endif /* INLCUDE_GDT_K */