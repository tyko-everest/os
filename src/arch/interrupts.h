#ifndef INCLUDE_INTERRUPTS_H
#define INCLUDE_INTERRUPTS_H

#include "clib/stddef.h"
#include "clib/stdint.h"
#include "system/proc.h"
#include "system/syscall.h"
#include "utils/printf.h"

// note: these numbers are hardcoded in vector.s
#define INT_SRC_Sync    0
#define INT_SRC_IRQ     1
#define INT_SRC_FIQ     2
#define INT_SRC_SError  3
#define INT_CURR_EL_SP0 (0 << 2)
#define INT_CURR_EL_SPx (1 << 2)
#define INT_LOW_EL_64   (2 << 2)
#define INT_LOW_EL_32   (3 << 2)

typedef enum {
    EC_UNKNOWN  = 0b000000,
    // ...
    EC_SVC64    = 0b010101,
} int_ec_t;

void interrupt_handler(uint64_t source, uint64_t esr, stacked_regs_t *reg);

#endif
