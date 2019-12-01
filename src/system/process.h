#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

// has the definition for the register state struct
#include "system/interrupts.h"

typedef struct {
    uint32_t pid;
    uint32_t page_desc_addr;
    stack_state_t registers;

} process_t;

#endif