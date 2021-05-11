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

// inline uint32_t read_sctlr() {
//     uint32_t ret;
//     asm volatile(
//         "mrc p15, 0, %[ret], c1, c0, 0"
//         : [ret] "=r" (ret)
//     );
//     return ret;
// }

// inline void write_sctlr(uint32_t val) {
//     asm volatile(
//         "mcr p15, 0, %[val], c1, c0, 0"
//         :
//         : [val] "r" (val)
//     );
// }

// inline uint32_t read_ttbcr() {
//     uint32_t ret;
//     asm volatile(
//         "mrc p15, 0, %[ret], c2, c0, 0"
//         : [ret] "=r" (ret)
//     );
//     return ret;
// }

// inline void write_ttbcr(uint32_t val) {
//     asm volatile(
//         "mcr p15, 0, %[val], c2, c0, 0"
//         :
//         : [val] "r" (val)
//     );
// }

inline uint64_t read_sctlr_el1() {
    uint32_t ret;
    asm volatile(
        "mrs %[ret], SCTLR_EL1"
        : [ret] "=r" (ret)
    );
    return ret;
}

#endif
