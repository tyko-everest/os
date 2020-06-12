#include "shell.h"

int cmd_entered = 0;

static fs_inode_t cur_inode;
static uint32_t cur_num;

void shell_init(void) {
    cur_inode = fs_get_inode(UXT_ROOT_INO);
    cur_num = UXT_ROOT_INO;
}

void shell_execute(void) {
    if (cmd_entered) {
        char cmd[16];
        scanf("%s", cmd);
        char arg[64];
        scanf("%s", arg);

        if (memcmp(cmd, "MKDIR", 5) == 0) {
            fs_mkfile(arg, cur_num, &cur_inode, UXT_FDIR, 0, 0);
        } else if (memcmp(cmd, "LS", 2) == 0) {
            fs_ls(&cur_inode);
        } else if (memcmp(cmd, "CD", 2) == 0) {
            fs_inode_t new_inode;
            uint32_t new_num = fs_get_file(arg, &cur_inode, &new_inode);
            if (new_num != 0) {
                cur_num = new_num;
                cur_inode = new_inode;
            }
        }
        cmd_entered = 0;
    }
}