#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

#include "print.h"

typedef struct __attribute__((packed)){
    unsigned short size;
    unsigned int address;
} gdt_header_t;

typedef struct __attribute__((packed)){
    unsigned short limit0_15;
    unsigned short base0_15;
    unsigned char base16_23;
    unsigned char flags;
    unsigned char flags_limit16_19;
    unsigned char base24_31;
} gdt_entry_t;

/** load_gdt:
 * Loads the specified gdt in assembly
 * @param gdt The address of a gdt struct
 */
void load_gdt(gdt_header_t gdt);

/** load_segment_registers
 * Loads all the segment registers with the proper segment descriptors
 */
void load_segment_registers(void);

void setup_flat_memory(void);

#endif /* INLCUDE_GDT_K */