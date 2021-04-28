#define TEST_FS

#include <stdlib.h>
#include <stdio.h>
#include "../src/system/ext2.h"

uint32_t ceil_div(uint32_t n, uint32_t d) {
    div_t result = div(n, d);
    if (result.rem == 0) {
        return result.quot;
    } else {
        return result.quot + 1;
    }
}

int main() {

    printf("%d\n", sizeof(ext2_inode_t));

    static uint8_t buf[1024];


    setup_fp();
    // load the sb into memory and calculate global vals from it
    fs_init();





    ext2_inode_t inode;
    uint32_t inum = 12;
    fs_get_inode(inum, &inode);

    // fs_ls(&inode);
    fs_mkfile("plswork", inum, &inode, EXT2_S_IFREG | EXT2_S_IRUSR, 1000, 1000);

    char str[1024];
    fs_readfile("/dir/plswork", 0, 1, str);

    ext2_inode_t boo, plswork;
    fs_get_inode(13, &boo);
    fs_get_inode(16, &plswork);

    return 0;
}