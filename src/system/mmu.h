#ifndef INCLUDE_MMU_H
#define INCLUDE_MMU_H

#include "clib/stdint.h"

/**
 * The Memory Management Unit
 * 
 * Plan:
 * - using short-descriptor translation table format
 * 
 */

uint64_t read_sctlr_el1();

#endif
