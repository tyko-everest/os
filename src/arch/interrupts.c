#include "interrupts.h"

static inline size_t get_ec(uint64_t esr) {
    return (esr >> 26) & 0b111111;
}

static inline size_t get_iss(uint64_t esr) {
    return esr & 0x1FFFFFF;
}

void interrupt_handler(uint64_t source, uint64_t esr) {
    printf("interrupt -> source: %d, esr: 0x%X\n", source, esr);

    size_t syscall;

    switch (source) {
    case INT_CURR_EL_SPx | INT_SRC_Sync:

        switch (get_ec(esr)) {
        case EC_SVC64:
            syscall = get_iss(esr) & 0xFFFF;
            printf("got syscall %d\n", syscall);
            break;
        
        default:
            printf("unhandled exception class: 0x%X", get_ec(esr));
            break;
        }
        
        break;
    
    default:
        printf("unhandled interrupt: source %d, esr 0x%X\n", source, esr);
        for(;;);
        break;
    }
}
