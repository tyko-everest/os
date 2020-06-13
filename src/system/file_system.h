#ifndef INCLUDE_FILE_SYSTEM_H
#define INCLUDE_FILE_SYSTEM_H

#include "macros.h"

// these are used when this file is being called inside the os
#ifndef TEST_FS

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include "system/kheap.h"

#include "ata.h"

#define SECTOR_SIZE ATA_BLOCK_SIZE

// these are used when using test_fs under linux
#else

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SECTOR_SIZE 512

void setup_fp(void);

#endif

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
#define UXT_MODE_MASK 0x0FFF
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
#define UXT_TYPE_MASK 0xF000
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
    // block if first data block
    uint32_t first_block;
    // counts for this block group
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint32_t RESERVED[3];
} fs_block_desc_t;

typedef struct __attribute((packed)) {
    // inode number of the file
    // if 0 this marks end of linked list
    uint32_t inode;
    // distance to next dir entry from start of this one
    // ensure if offset goes into next block, that you load the next block
    uint16_t offset;
    uint8_t name_len;
    uint8_t PADDING;
    // immediately after this struct the bytes for the name should start
} fs_dir_entry_t;

// TODO: most of these functions are hardcoded to 1 KiB buffers for now
// needs to change to support different block sizes eventually

// abstract away what interface actually gets the data
// by default it compiled to use ata driver inside os
// but defining TEST_FS compiles to use normal c file operations
// used for testing the file system outside the os
void read_block(uint32_t block_id, uint8_t *buf);
void write_block(uint32_t block_id, uint8_t *buf);

// reads superblock into memory and calculates some global numbers
// that are used in calculations in functions below
void fs_init(void);


// helper functions

// load the block group descriptor for the block that contains this inode
// also returns the block group number, starting from 0
uint32_t fs_load_bgd(uint32_t inode_num, fs_block_desc_t *ret_bgd);
// get or set an inode by number
fs_inode_t fs_get_inode(uint32_t inode_num);
void fs_set_inode(uint32_t inode_num, const fs_inode_t * inode);
// finds first free bit in bitmap, used with either inode or block bitmaps
// first bit is 0th bit
// also sets the the bit to 1 to mark it as used
// returns -1 if none found and no bits changed
int32_t fs_find_free(uint32_t block_id);

// setup a blank directory with . and .. dirs in it initially
// for the given inode at its first block
// @params blank_num and blank_inode are for the directory being setup
// dir_num is for its parent
void fs_setup_blank_dir(uint32_t blank_num, const fs_inode_t *blank_inode,
    uint32_t parent_num);

// for the given inode turns a block_list index into a block id
// necessary for dealing with indirect pointers after first 12 direct ones
uint32_t fs_get_block_id(uint32_t block_index, const fs_inode_t *inode);
// sets the desired part of the block pointer structure to the desired block id
int fs_set_block_id(uint32_t block_id, uint32_t block_index, fs_inode_t *inode);

// given a path it returns the inode number and inode struct
// returns 0 if the file does not exist
uint32_t fs_path_to_inode(const char *path, fs_inode_t *ret_inode);


// file system operation functions

void fs_ls(const fs_inode_t *dir_inode);

// given a file name, and directory inode, returns the inode num and struct
// TODO, this may not need to exist, only used by shell because it keeps track
// of working dirs inode
uint32_t fs_get_file(const char *name, const fs_inode_t *dir_inode,
    fs_inode_t *ret_inode);

// makes a file
// note dir is modified by strtok, change later if 
int fs_mkfile(const char *name, uint32_t dir_num, const fs_inode_t *dir_inode,
    uint16_t mode, uint16_t uid, uint16_t gid);

int fs_rmfile(const char *name, uint32_t dir_num, const fs_inode_t *dir_inode);

// read num blocks starting at start of the specified file
// return number of blocks read, or -1 if error
int fs_readfile(const char *path, uint32_t start, uint32_t num, uint8_t *buf);

// write num blocks starting at start of the specified file
// return number of blocks read, or -1 if error
int fs_writefile(const char *path, uint32_t start, uint32_t num, uint8_t *buf);

#endif // INCLUDE_FILE_SYSTEM_H