#include "ustar.h"

int oct2bin(const char *s, int len) {
    int n = 0;
    while (len > 0) {
        n = n << 3;
        n += *s - '0';
        s++;
        len--;
    }
    return n;
}

int open(const char *pathname, int flags) {
    ustar_meta_t meta;
    ata_pio_read(ATA0, ATA_MASTER, 0, 1, &meta);
    if (strcmp(meta.name, pathname) == 0) {
        
    }
    
}
