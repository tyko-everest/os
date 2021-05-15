#include "mmu.h"

inline uint64_t read_sctlr_el1() {
    uint64_t ret;
    asm volatile(
        "mrs %[ret], SCTLR_EL1"
        : [ret] "=r" (ret)
    );
    return ret;
}
