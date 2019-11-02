#include "gdt.h"

static gdt_entry_t gdt_table[5] = {
    {0, 0, 0, 0, 0, 0},
    {0xFFFF, 0, 0, 0b10011010, 0b11001111, 0},
    {0xFFFF, 0, 0, 0b10010010, 0b11001111, 0},
    {0xFFFF, 0, 0, 0b11111010, 0b11001111, 0},
    {0xFFFF, 0, 0, 0b11110010, 0b11001111, 0}
    
};

gdt_header_t gdt = {5 * sizeof(gdt_entry_t) - 1, (unsigned int) gdt_table};

void setup_flat_memory() {
    load_gdt(gdt);
    load_segment_registers();
}