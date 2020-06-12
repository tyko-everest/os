#include <stdlib.h>
#include <stdio.h>
#include "../src/system/file_system.h"

uint32_t ceil_div(uint32_t n, uint32_t d) {
    div_t result = div(n, d);
    if (result.rem == 0) {
        return result.quot;
    } else {
        return result.quot + 1;
    }
}

// this used to prep a uxt formatted disk file for use in bochs
void make_fs() {

    // to get the num bytes just use ls -l on disk.img
    // gives correct value as verified by ata identify command
    // this is bytes for a 256 MiB disk
    uint32_t num_bytes = 268435456;
    uint32_t sector_size = 512;
    uint32_t num_sectors = num_bytes / sector_size;

    fs_super_block_t sb;

    // configure these for formatting
    sb.log_block_size = 0;
    // but not this one
    uint32_t block_size = 1024 << sb.log_block_size;    
    // this is the maxiumum number of blocks per block group, since bitmaps
    // need to fit into one block
    sb.blocks_per_group = block_size * 8; // 8 bits per byte, for bitmap
    // inodes could be less than this, or up to 8 times more
    sb.inodes_per_group = block_size;

    // total num of blocks available on the partition
    uint32_t num_blocks_possible = num_sectors / (block_size / sector_size);
    // how many inodes can fit in a block
    uint32_t inodes_per_block = block_size / sizeof(fs_inode_t);
    // how many blocks to fit the inode table in each block group
    uint32_t inode_table_block_size = sb.inodes_per_group / inodes_per_block;
    // number of blocks per group that are used for metadata
    // two bitmap blocks + blocks used for inode table
    uint32_t meta_blocks_per_group = 2 + inode_table_block_size;
    // all blocks needed for a block group, incl meta
    uint32_t total_blocks_per_group =
        sb.blocks_per_group + meta_blocks_per_group;

    sb.blocks_count = 0;
    sb.inodes_count = 0;
    sb.block_groups_count = 0;
    // number of block group desc entries that can fit in one block
    uint32_t bgd_per_block = block_size / sizeof(fs_block_desc_t);
    // used to find number of blocks the bgdt will take up
    uint32_t blocks_for_bgdt = 0;
    // used to deal with oddly sized last block group
    uint32_t blocks_in_last_bg = sb.blocks_per_group;
    // start with one less due to superblock
    uint32_t num_blocks_remaining = num_blocks_possible - 1;
    while (num_blocks_remaining != 0) {
        // check if we have made enough block groups that a new block
        // is needed for the block group desc table
        if (sb.block_groups_count % bgd_per_block == 0) {
            blocks_for_bgdt++;
            num_blocks_remaining--;
        }
        // add a block group if enough space for at least metadata
        if (num_blocks_remaining >= meta_blocks_per_group) {
            sb.block_groups_count++;
            sb.inodes_count += sb.inodes_per_group;
            num_blocks_remaining -= meta_blocks_per_group;
            // check if this is a full block or not
            // amount of blocks added could change
            if (num_blocks_remaining >= sb.blocks_per_group) {
                sb.blocks_count += sb.blocks_per_group;
                num_blocks_remaining -= sb.blocks_per_group;
            } else {
                blocks_in_last_bg = num_blocks_remaining;
                sb.blocks_count += num_blocks_remaining;
                num_blocks_remaining = 0;             
            }
        // if it's not enough it will just be wasted space
        } else {
            break;
        }
    }
    // all start free
    sb.free_blocks_count = sb.blocks_count;
    sb.free_inodes_count = sb.inodes_count;

    // open disk.img 
    FILE *fp;
    fp = fopen("../disk.img", "w");

    // write superblock to first page of disk
    fwrite(&sb, 1, sizeof(fs_super_block_t), fp);

    // skip to next block if necessary (i.e. not 1 KiB blocks)
    fseek(fp, block_size - sizeof(fs_super_block_t), SEEK_CUR);
    
    // save this pos so we can seek cleanly to next free block
    fpos_t bgdt_pos;
    fgetpos(fp, &bgdt_pos);

    // fill in metadata for all block group desc entries
    // except for last one which may not be full
    fs_block_desc_t block_desc;
    block_desc.free_blocks_count = sb.blocks_per_group;
    block_desc.free_inodes_count = sb.inodes_per_group;
    for (size_t i = 0; i < sb.block_groups_count - 1; i++) {
        block_desc.block_bitmap =
            1 + blocks_for_bgdt + i * total_blocks_per_group;
        block_desc.inode_bitmap = block_desc.block_bitmap + 1;
        block_desc.inode_table = block_desc.inode_bitmap + 1;
        block_desc.first_block = block_desc.inode_table + inode_table_block_size;
        fwrite(&block_desc, 1, sizeof(fs_block_desc_t), fp);
    }
    // handle last block group desc entry, potentially non-full
    block_desc.free_blocks_count = blocks_in_last_bg;
    fwrite(&block_desc, 1, sizeof(fs_block_desc_t), fp);

    // go back and seek forward to land on a block interval
    fsetpos(fp, &bgdt_pos);
    fseek(fp, blocks_for_bgdt * block_size, SEEK_CUR);

    // fill in bitmaps for the block groups, except last one
    uint8_t zeros_byte = 0;
    for (size_t i = 0; i < sb.block_groups_count - 1; i++) {
        // write blocks and inodes bitmaps
        for (size_t j = 0; j < block_size * 2; j++) {
            fwrite(&zeros_byte, 1, 1, fp);
        }
        // skip over inodes table and blocks
        fseek(fp, block_size * (sb.blocks_per_group + inode_table_block_size),
            SEEK_CUR);
    }

    // last one might not be full size, so need to mark any blocks that
    // don't actually exist as used
    uint8_t ones_byte = 0xFF;
    // byte that contains point where blocks start being marked as used
    uint8_t divider_byte = 0xFF >> (blocks_in_last_bg % 8);
    // what byte this occurs at in the block
    uint32_t divider_index = blocks_in_last_bg / 8;
    // now write the last block bitmap
    for (size_t i = 0; i < divider_index; i++) {
        fwrite(&zeros_byte, 1, 1, fp);
    }
    fwrite(&divider_byte, 1, 1, fp);
    for (size_t i = 0; i < block_size - divider_index - 1; i++) {
        fwrite(&ones_byte, 1, 1, fp);
    }
    // inode bitmap is always normal size even in last block group
    for (size_t i = 0; i < block_size; i++) {
        fwrite(&zeros_byte, 1, 1, fp);
    }

    // this part is only needed because writing to a file only makes it as big
    // as necessary, unlike a partition which would be a fixed size
    // so we need to force it to be the 256 MiB it's supposed to be

    // seek past the last inode table
    fseek(fp, inode_table_block_size * block_size, SEEK_CUR);
    // seek past the remaining blocks of the last block group minus one byte
    fseek(fp, blocks_in_last_bg * block_size - 1, SEEK_CUR);
    fwrite(&zeros_byte, 1, 1, fp);

    fclose(fp);
}

// this is for testing
// some values are harcoded for a block size of 1 KiB
// most block ids are harcoded too
void setup_root_dir() {

    uint8_t buf[1024];

    uint16_t root_mode =
        UXT_RUSR | UXT_WUSR | UXT_XUSR | UXT_RGRP | UXT_XGRP |
        UXT_ROTH | UXT_XOTH | UXT_FDIR;

    // setup root inode to point to directory entry list
    fs_inode_t root_inode = {
        .mode = root_mode,
        .links = 2,
        .user = 0,
        .group = 0,
        .blocks_count = 1,
        .block_list[0] = 132
    };
    fs_set_inode(1, &root_inode);

    // set inode 1 as used
    memset(buf, 0, 1024);
    buf[0] = 1;
    write_block(3, buf);
    // set first block as used
    write_block(2, buf);


    fs_setup_blank_dir(UXT_ROOT_INO, &root_inode, UXT_ROOT_INO);

}

int main() {
    make_fs();
    // setup file pointer to write to the disk.img file
    setup_fp();
    // load the sb into memory and calculate global vals from it
    fs_init();
    // setup the root directory
    setup_root_dir();


    return 0;
}