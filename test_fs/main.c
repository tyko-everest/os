
#include <stdio.h>
#include "../src/system/fat32.h"


int main() {

    static uint8_t buf[2000] = "LILILILILILI";
    uint32_t len = strlen(buf);

    setup_sector();
    fat32_init();

    #define DIR "/TEST.TXT"
    char path[] = DIR;
    int path_len = sizeof(path);

    int write_len = fat32_writefile(path, 512, len, buf);

    memset(buf, 0, 2000);
    memcpy(path, DIR, path_len);

    int read_len = fat32_readfile(path, 512, 2000, buf);

    printf("%.*s\n", read_len, buf);

    return 0;
}