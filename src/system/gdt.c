#include "gdt.h"

static tss_t tss0;

static gdt_entry_t gdt_table[GDT_SIZE];

gdt_header_t gdt_info = {GDT_SIZE * sizeof(gdt_entry_t) - 1, (unsigned int) gdt_table};

extern unsigned int kernel_stack_start;

void add_gdt_entry(uint32_t limit, uint32_t base, uint8_t access_byte,
                   uint8_t flags) {
    
    static uint32_t index = 0;

    gdt_table[index].limit0_15 = limit & 0xFFFF;
    gdt_table[index].base0_15 = base & 0xFFFF;
    gdt_table[index].base16_23 = (base >> 16) & 0xFF;
    gdt_table[index].access_byte = access_byte;
    gdt_table[index].flags_limit16_19 =
            (flags & 0b11000000) | ((limit >> 16) & 0xFF);
    gdt_table[index].base24_31 = (base >> 24) & 0xFF;

    index++;
}

void setup_flat_memory() {
    // null descriptor
    add_gdt_entry(0, 0, 0, 0);
    // kernel code
    add_gdt_entry(0xFFFFF, 0, 0b10011010, 0b11000000);
    // kernel data
    add_gdt_entry(0xFFFFF, 0, 0b10010010, 0b11000000);
    // user code
    add_gdt_entry(0xFFFFF, 0, 0b11111010, 0b11000000);
    // user data
    add_gdt_entry(0xFFFFF, 0, 0b11110010, 0b11000000);

    gdt_header_t test;

    load_gdt(gdt_info);
    load_segment_registers();
}

void setup_tss(uint32_t kernel_stack_base) {
    tss0.ESP0 = kernel_stack_base;
    tss0.SS0 = 0x10;
    tss0.IOPB = sizeof(tss_t);
    // task state segment for sys calls
    add_gdt_entry(sizeof(tss_t), &tss0, 0x89, 0x40);
    load_tss(tss0);
}