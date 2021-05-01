
#include <stdio.h>
#include "../src/system/fat32.h"


int main() {

    static uint8_t buf[2000] = "ALLOO Me dudesss";
    uint32_t len = strlen(buf);

    setup_sector();
    fat32_init();

    #define DIR "/BOO2"
    char path[] = DIR;
    int path_len = sizeof(path);

    int res = fat32_makefile(path, false);

    memcpy(path, DIR, path_len);

    int write_len = fat32_writefile(path, 0, len, buf);

    memset(buf, 0, 2000);
    memcpy(path, DIR, path_len);

    int read_len = fat32_readfile(path, 0, 100, buf);

    printf("%.*s\n", read_len, buf);
    memcpy(path, DIR, path_len);
    
    int res1 = fat32_deletefile(path);


    return 0;
}