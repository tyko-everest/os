#ifndef INCLUDE_FILE_SYSTEM_H
#define INCLUDE_FILE_SYSTEM_H

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "string.h"

// TODO put an abstraction layer between the driver and the file system
#include "ata.h"

/*
Header for uxt, the Unextented File System

Example layout with 1 KiB block size

block 0: superblock
block 1: block group desc table
block 2: block bitmap
block 3: inode bitmap
block 4: inode table (calculated based on inodes per group)
block N: data blocks

this would then repeat, possibly with superblock and block group desc backup
*/

#define FS_SMALLEST_BLOCK_SIZE 1024

#define UXT_ROOT_UID 0
#define UXT_ROOT_GID 0

// reserved inode numbers
#define UXT_ROOT_INO 1

// file modes
#define UXT_XOTH 0x0001
#define UXT_WOTH 0x0002
#define UXT_ROTH 0x0004
#define UXT_XGRP 0x0008
#define UXT_WGRP 0x0010
#define UXT_RGRP 0x0020
#define UXT_XUSR 0x0040
#define UXT_WUSR 0x0080
#define UXT_RUSR 0x0100
// file types
#define UXT_FREG 0x1000
#define UXT_FDIR 0x2000
#define UXT_FLNK 0x3000

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
    // 12 direct block id pointers, then single, double, and triple pointer
    uint32_t block_list[15];
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

typedef struct __attribute((packed)) {
    // inode number of the file
    uint32_t inode;
    // distance to next dir entry from start of this one
    // since directory entries must be 4-byte aligned last 2 bit are free
    // bit 0: last_rec, if set, indicates this is the last entry
    // bit 1: next_block, if set, next entry is is the next block,
    // and the offset starts from beginning of next block,
    // i.e. if next entry is at very beginning of next block, offset = 0
    uint16_t offset;
    uint8_t name_len;
    uint8_t PADDING;
    // immediately after this struct the bytes for the name should start
} fs_dir_entry_t;

void read_block(uint32_t block_id, uint8_t *buf);
void write_block(uint32_t block_id, uint8_t *buf);
void fs_init(void);

fs_inode_t fs_get_inode(uint32_t inode_num);
void fs_set_inode(uint32_t inode_num, fs_inode_t inode);



#endif // INCLUDE_FILE_SYSTEM_H