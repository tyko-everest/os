#include "file_system.h"

static fs_super_block_t sb;
static uint32_t block_size;
// to convet block num to sector num
// default to 1 KiB block sizes, also necessary to read in super block
// with read_block function
static uint32_t spb = 2;
// number of block group descriptors that can fit in one block
static uint32_t bgd_per_block;
static uint32_t inodes_per_block;

// TODO make this dynamic at some point
// block buffer to load a block into memory
static uint8_t bb[1024];

void read_block(uint32_t block_id, uint8_t *buf) {
    ata_pio_read(ATA0, ATA_MASTER, block_id * spb, spb, buf);
}

void write_block(uint32_t block_id, uint8_t *buf) {
    ata_pio_write(ATA0, ATA_MASTER, block_id * spb, spb, buf);
}

void fs_init(void) {
    read_block(0, &sb);
    block_size = 1024 << sb.log_block_size;
    spb = block_size / ATA_BLOCK_SIZE;
    bgd_per_block = block_size / sizeof(fs_block_desc_t);
    inodes_per_block = block_size / sizeof(fs_inode_t);
}


fs_inode_t fs_get_inode(uint32_t inode_num) {
    fs_inode_t inode;
    // check to see if inode exists on the file system
    if (inode_num > sb.inodes_count || inode_num == 0) {
        memset(&inode, 0, sizeof(fs_inode_t));
        return inode;
    }

    // which block of the block group desc table has the needed entry
    uint32_t bgdt_block_id =
        (inode_num - 1) / (bgd_per_block * sb.inodes_per_group); 
    // add one for superblock
    read_block(bgdt_block_id + 1, bb);
    // which entry on the selected block holds the right block group entry
    fs_block_desc_t bgd;
    uint32_t bgd_num = (inode_num - 1) / sb.inodes_per_group;
    memcpy(&bgd, ((fs_block_desc_t *) bb) + bgd_num,
        sizeof(fs_block_desc_t));

    // get the correct part of the inode table
    read_block(bgd.inode_table + (inode_num - 1) % inodes_per_block, bb);
    // get the correct inode
    memcpy(&inode, ((fs_inode_t *) bb) + (inode_num - 1) % inodes_per_block,
        sizeof(fs_inode_t));

    return inode;
}

void fs_set_inode(uint32_t inode_num, fs_inode_t inode) {
    // check to see if inode exists on the file system
    if (inode_num > sb.inodes_count || inode_num == 0) {
        memset(&inode, 0, sizeof(fs_inode_t));
        return;
    }

    // which block of the block group desc table has the needed entry
    uint32_t bgdt_block_id =
        (inode_num - 1) / (bgd_per_block * sb.inodes_per_group); 
    // add one for superblock
    read_block(bgdt_block_id + 1, bb);
    // which entry on the selected block holds the right block group entry
    fs_block_desc_t bgd;
    uint32_t bgd_num = (inode_num - 1) / sb.inodes_per_group;
    memcpy(&bgd, ((fs_block_desc_t *) bb) + bgd_num,
        sizeof(fs_block_desc_t));

    // get the correct part of the inode table
    read_block(bgd.inode_table + (inode_num - 1) % inodes_per_block, bb);
    // get the correct inode
    memcpy(((fs_inode_t *) bb) + (inode_num - 1) % inodes_per_block, &inode,
        sizeof(fs_inode_t));
    write_block(bgd.inode_table + (inode_num - 1) % inodes_per_block, bb);
}
