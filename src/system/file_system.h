#ifndef INCLUDE_FILE_SYSTEM_H
#define INCLUDE_FILE_SYSTEM_H

#include "stdint.h"

/*
Header for uxt, the Unextented File System

*/

/*
Example layout with 1 KiB block size

block 0: superblock
block 1: block group desc table
block 2: block bitmap
block 3: inode bitmap
block 4: inode table (calculated based on inodes per group)
block N: data blocks

this would then repeat, possibly with superblock and block group desc backup
*/

#define FS_SUPER_BLOCK_SIZE 1024
#define FS_SMALLEST_BLOCK_SIZE 1024

typedef uint32_t * fs_block_ptr_t;

typedef struct {
    uint32_t device_id;
    uint32_t serial_num;
    uint16_t mode;
    uint16_t link_count;
    uint16_t user_id;
    uint16_t group_id;
    uint32_t dev_id;
    uint32_t size;
    // inode change time
    uint32_t ctime;
    // file content change time
    uint32_t mtime;
    // last access time
    uint32_t atime;
    uint32_t io_block_size;
    uint32_t num_blocks;
} posix_inode_t;

// not going to go full POSIX anytime soon
// 128 bytes like ext2 inodes
typedef struct __attribute((packed)) {
    uint16_t mode;
    uint16_t links;
    uint16_t user;
    uint16_t group;
    // inode change time
    uint32_t ctime;
    // file content change time
    uint32_t mtime;
    // last access time
    uint32_t atime;
    uint32_t blocks_count;
    uint32_t size_low;
    uint32_t size_high;
    // following the UFS pointer structure
    // 12 direct block pointers, then single, double, and triple pointer
    fs_block_ptr_t block_list[15];
    uint32_t RESERVED[9];
} fs_inode_t;

typedef struct __attribute((packed)) {
    // total num on file system
    uint32_t inodes_count;
    // blocks that can be used for storage, don't incl ones used for fs
    uint32_t blocks_count;
    // num free on file system
    uint32_t free_inodes_count;
    uint32_t free_blocks_count;
    // num per block group
    uint32_t inodes_per_group;
    uint32_t blocks_per_group;
    // block_size = 1024 << log_block_size
    uint32_t log_block_size;
    uint32_t block_groups_count;
    // fixed to 1 KiB
    uint32_t RESERVED[248];
} fs_super_block_t;

typedef struct __attribute((packed)) {
    // block id of block that holds used blocks bitmap
    uint32_t block_bitmap;
    // block id of block that holds used inodes bitmap
    uint32_t inode_bitmap;
    // block id of first block of inode table
    uint32_t inode_table;
    // counts for this block group
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
} fs_block_desc_t;

void mkfs() {

}

#endif // INCLUDE_FILE_SYSTEM_H