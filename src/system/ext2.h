#ifndef INCLUDE_EXT2_H
#define INCLUDE_EXT2_H

// these are used when this file is being called inside the os
#ifndef TEST_FS

#include "macros.h"
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

#include "../utils/macros.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SECTOR_SIZE 512

void* kmalloc(unsigned int bytes);
void kfree(void *ptr);

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
#define EXT2_ROOT_INO 2

// file modes
#define EXT2_MODE_MASK 0x0FFF
#define EXT2_S_IXOTH 0x0001
#define EXT2_S_IWOTH 0x0002
#define EXT2_S_IROTH 0x0004
#define EXT2_S_IXGRP 0x0008
#define EXT2_S_IWGRP 0x0010
#define EXT2_S_IRGRP 0x0020
#define EXT2_S_IXUSR 0x0040
#define EXT2_S_IWUSR 0x0080
#define EXT2_S_IRUSR 0x0100

// file types
#define EXT2_TYPE_MASK 0xF000
#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFDIR 0x4000

typedef struct {
    uint32_t device_id;
    uint32_t serial_num;
    uint16_t i_mode;
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
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    // last access time
    uint32_t i_atime;
    // inode creation time
    uint32_t i_ctime;
    // inode modify time
    uint32_t i_mtime;
    // inode delete time
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    // number of currently reserved blocks for this file
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    // following the UFS pointer structure
    // 12 direct block id pointers, then single, double, and triple pointer
    // note: these are 512-byte blocks
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint32_t i_osd2[3];
} ext2_inode_t;

typedef struct __attribute((packed)) {
    // total num on file system
    uint32_t s_inodes_count;
    // blocks that can be used for storage, don't incl ones used for fs
    uint32_t s_blocks_count;
    // blocks reserved for the super user
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    // block id of first super block, i.e. 0 or 1
    uint32_t s_first_data_block;
    // block_size = 1024 << log_block_size
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    // fixed to 1 KiB
    uint8_t RESERVED[940];
} ext2_super_block_t;

typedef struct __attribute((packed)) {
    // block id of block that holds used blocks bitmap
    uint32_t bg_block_bitmap;
    // block id of block that holds used inodes bitmap
    uint32_t bg_inode_bitmap;
    // block id of first block of inode table
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint8_t RESERVED[12];
} ext2_block_desc_t;

typedef struct __attribute((packed)) {
    // inode number of the file
    // if 0 this marks end of linked list
    uint32_t inode;
    // distance to next dir entry from start of this one
    // ensure if rec_len goes into next block, that you load the next block
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    // immediately after this struct the bytes for the name should start
} ext2_dir_entry_t;

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
int fs_init(void);


/* helper functions */

/**
 * Loads a block group descriptor into memory
 * @param inode_num the returned descriptor will be for the block group that
 * contains this inode
 * @param ret_bgd a pointer to a ext2_block_desc_t, used to return the
 * actual descriptor
 * @return the number of the block group which contains the inode,
 * or a negative value if there was an error
 */
uint32_t fs_load_bgd(uint32_t inode_num, ext2_block_desc_t *ret_bgd);

/**
 * Gets an inode struct into memory
 * @param inode_num the number of the inode
 * @param ret_inode a pointer to an ext2_inode_t, the requested inode will
 * be returned here
 * @return positive inode number, 0 if error
 */
uint32_t fs_get_inode(uint32_t inode_num, ext2_inode_t *inode);

/**
 * Sets an inode on the disk
 * @param inode_num the number of the inode
 * @param ret_inode a pointer to a const ext2_inode_t, the disk will be
 * written with its value
 * @return positive inode number, 0 if error
 */
uint32_t fs_set_inode(uint32_t inode_num, const ext2_inode_t *inode);

/**
 * Finds the first free bit given a bitmap in a block
 * @note
 * Works for both inode and block bitmaps
 * Sets the bit as taken on the disk, so make sure to keep track of it
 * @param block_id the block number on the disk which has the bitmap
 * @return the number of the first free bit, or negative if error
 */
int32_t fs_find_free(uint32_t block_id);

/**
 * Dereferences the UFS block pointer structure for an inode
 * @param block_index the number of the desired block
 * @param inode a pointer to the constant inode struct
 * @return the block id of the desired block, or 0 if error
 */
uint32_t fs_get_block_id(uint32_t block_index, const ext2_inode_t *inode);

/**
 * Sets the appropriate value in the UFS block pointer structure for an inode
 * @param block_id the new value at the desired point in the structure
 * @param block_index the index of the desired block to change
 * @param inode a pointer to the constant inode struct
 * @return error code, negative for error
 */
int fs_set_block_id(uint32_t block_id, uint32_t block_index, ext2_inode_t *inode);

/**
 * Sets up the first block of a new directory with . and .. dirs
 * @param blank_num inode number for the directory file
 * @param blank_inode inode for the directory file
 * @param parent_num inode number of parent directory
 */
void fs_setup_blank_dir(uint32_t blank_num, const ext2_inode_t *blank_inode,
    uint32_t parent_num);

// given a path it returns the inode number and inode struct
// returns 0 if the file does not exist
uint32_t fs_path_to_inode(const char *path, ext2_inode_t *ret_inode);


// file system operation functions

void fs_ls(const ext2_inode_t *dir_inode);

/**
 * Gets the inode given a file name and the inode of the directory to search
 * @param name file name
 * @param dir_inode pointer to inode of directory
 * @param ret_inode where the found inode is returned
 * @return inode number, 0 indicates error
 */
uint32_t fs_get_file(const char *name, const ext2_inode_t *dir_inode,
    ext2_inode_t *ret_inode);

// makes a file
// note dir is modified by strtok, change later if 
int fs_mkfile(const char *name, uint32_t dir_num, const ext2_inode_t *dir_inode,
    uint16_t i_mode, uint16_t uid, uint16_t gid);

int fs_rmfile(const char *name, uint32_t dir_num, const ext2_inode_t *dir_inode);

// read num blocks starting at start of the specified file
// return number of blocks read, or -1 if error
int fs_readfile(const char *path, uint32_t start, uint32_t num, uint8_t *buf);

// write num blocks starting at start of the specified file
// return number of blocks read, or -1 if error
int fs_writefile(const char *path, uint32_t start, uint32_t num, uint8_t *buf);

#endif // INCLUDE_FILE_SYSTEM_H