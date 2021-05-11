#include "mmio.h"

inline uint32_t mmio_read(mmio_t reg) {
    return *((volatile uint32_t *) reg);
}
inline void mmio_write(mmio_t reg, uint32_t val) {
    *((volatile uint32_t *) reg) = val;
}