#include "string.h"

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *dest_ptr = (uint8_t *) dest;
    uint8_t *src_ptr = (uint8_t *) src;
    if (dest < src) {
        for (size_t i = 0; i < n; i++) {
            dest_ptr[i] = src_ptr[i];
        }
    } else if (dest > src) {
        for (size_t i = n; i > 0; i--) {
            dest_ptr[i - 1] = src_ptr[i - 1];
        }
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *dest_ptr = (uint8_t *) dest;
    uint8_t *src_ptr = (uint8_t *) src;
    if (dest < src) {
        for (size_t i = 0; i < n; i++) {
            dest_ptr[i] = src_ptr[i];
        }
    } else if (dest > src) {
        for (size_t i = n; i > 0; i--) {
            dest_ptr[i - 1] = src_ptr[i - 1];
        }
    }
    return dest;
}

void *memset(void *str, int c, size_t n) {
    uint8_t *str_ptr = (uint8_t *) str;
    for (size_t i = 0; i < n; i++) {
        str_ptr[i] = (uint8_t) c;
    }
    return str;
}
