#include "interrupts.h"

static inline size_t get_ec(uint64_t esr) {
    return (esr >> 26) & 0b111111;
}

static inline size_t get_iss(uint64_t esr) {
    return esr & 0x1FFFFFF;
}

void interrupt_handler(uint64_t source, uint64_t esr, general_regs_t *reg) {
    uint64_t elr;
    asm (
        "mrs %[elr], ELR_EL1"
        : [elr] "=r" (elr)
    );
    #ifdef DEBUG
    printf("interrupt -> source: %d, esr: 0x%X, elr: 0x%X\n", source, esr, elr);
    #endif

    size_t syscall;
    uint32_t stat;

    // which entry in the vector table did this come from
    switch (source) {
    case INT_CURR_EL_SPx | INT_SRC_Sync:
    case INT_LOW_EL_64 | INT_SRC_Sync:

        // which synchronous exception code was given
        switch (get_ec(esr)) {
        case EC_SVC64:
            syscall = get_iss(esr) & 0xFFFF;
            printf("got syscall %d\n", syscall);

            // route to the proper syscall
            switch (syscall) {
            case SYS_READ:
                reg->x[0] = sys_read((const char *) reg->x[0], (char *) reg->x[1], (size_t) reg->x[2], (size_t) reg->x[3]);
                break;

            case SYS_EXEC:
                sys_exec((const char *) reg->x[0]);
                break;
            
            case SYS_PRINT:
                sys_print((const char *) reg->x[0]);
                break;

            default:
                printf("unhandled syscall: %d\n", syscall);
                for(;;);
                break;
            }
            break;
        
        default:
            printf("unhandled exception class: 0x%X\n", get_ec(esr));
            for(;;);
            break;
        }
        
        break;

    case INT_CURR_EL_SPx | INT_SRC_IRQ:
    case INT_LOW_EL_64 | INT_SRC_IRQ:
        // this is currently not checking for which kind of IRQ we got
        // only enabled one is the UART at the moment
        #ifdef DEBUG
        printf("got IRQ\n");
        #endif
        stat = mmio_read(AUX_MU_STAT_REG);
        size_t rec = (stat >> 16) & 0xF;
        while (rec > 0) {
            char c = mmio_read(AUX_MU_IO_REG);
            if (c == '\r') {
                c = '\n';
            }
            printf("%c", c);
            rec--;
        }
        break;
    
    default:
        printf("unhandled interrupt source\n");
        for(;;);
        break;
    }
}
