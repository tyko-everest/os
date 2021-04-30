
#include <stdio.h>
#include "../src/system/fat32.h"


int main() {

    #define LEN 100
    static uint8_t buf[LEN];
    memset(buf, 'A', 100);

    int i = sizeof(fat32_vol_id_t);

    setup_sector();
    fat32_init();

    char path[] = "/DIR1/DIR2/HELLO";
    int test = fat32_readfile(path, 0, LEN, buf);

    printf("%.*s\n", test, buf);

    return 0;
}