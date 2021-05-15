#include "registers.h"

inline uint64_t read_current_el() {
    uint64_t ret;
    asm volatile(
        "mrs %[ret],  CurrentEL"
        : [ret] "=r" (ret)
    );
    return ret;
}