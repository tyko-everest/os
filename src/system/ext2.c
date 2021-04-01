#include "ext2.h"

static ext2_super_block_t sb;
// spb: sectors per block
// to convert block num to sector num
static uint32_t spb = 2;
// default to 1 KiB block sizes, also necessary to read in super block
// with read_block function
static uint32_t block_size = 1024;
// number of block group descriptors that can fit in one block
static uint32_t bgd_per_block;
static uint32_t inodes_per_block;


// various constants that are calculated from the superblock on initilization
// these will not change in operation

static uint32_t blocks_per_bgdt;
// total blocks used for meta data per block group
// i.e. bdgt + block bitmap + inode bitmap
static uint32_t meta_blocks_per_bg;

#ifndef TEST_FS

void read_block(uint32_t block_id, uint8_t *buf) {
    ata_pio_read(ATA0, ATA_MASTER, block_id * spb, spb, (uint16_t*) buf);
}

void write_block(uint32_t block_id, uint8_t *buf) {
    ata_pio_write(ATA0, ATA_MASTER, block_id * spb, spb, (uint16_t*) buf);
}

#else

FILE *fp;

void setup_fp() {
    fp = fopen("/home/tyko/os/disk.img", "r+");
}

void read_block(uint32_t block_id, uint8_t *buf) {
    fseek(fp, block_id * block_size, SEEK_SET);
    fread(buf, 1, block_size, fp);
}

void write_block(uint32_t block_id, uint8_t *buf) {
    fseek(fp, block_id * block_size, SEEK_SET);
    fwrite(buf, 1, block_size, fp);
}

void* kmalloc(unsigned int bytes) {
    return malloc(bytes);
}

void kfree(void *ptr) {
    free(ptr);
}

#endif

int fs_init() {
    // setup only works with 1 KiB blocks currently
    read_block(1, (uint8_t *) &sb);

    // check to ensure this is a compatible ext2 file system
    if (sb.s_rev_level != 0) {
        return -1;
    }

    block_size = 1024 << sb.s_log_block_size;
    spb = block_size / SECTOR_SIZE;
    bgd_per_block = block_size / sizeof(ext2_block_desc_t);
    inodes_per_block = block_size / sizeof(ext2_inode_t);

    uint32_t num_bgs = CEIL(sb.s_blocks_count, sb.s_blocks_per_group);
    blocks_per_bgdt = CEIL(num_bgs, sizeof(ext2_block_desc_t));
    // superblock + bgdt + block bitmap + inode bitmap + inode table
    meta_blocks_per_bg = 1 + blocks_per_bgdt + 1 + 1;

    return 0;
}

uint32_t fs_load_bgd(uint32_t inode_num, ext2_block_desc_t *ret_bgd) {
    uint8_t buf[1024];

    // determine which block group this inode is in
    uint32_t bg_num = (inode_num - 1) / sb.s_inodes_per_group;

    // which block of the block group desc table has the needed entry
    // possible for block group desc table to take up multiple blocks
    uint32_t bgdt_block =
        (inode_num - 1) / (bgd_per_block * sb.s_inodes_per_group); 
    // add first_dat_block to account for where fist super block starts
    // add 1 because block group decripter table always block after superblock
    // which would be the 0th block in the block group
    read_block(bgdt_block + sb.s_first_data_block + 1, buf);

    // with the needed part of the bgdt table loaded
    // treating it as an array of bgd, what is the index
    // need remainder because bgdt can take up multiple blocks
    uint32_t bgd_entry = ((ext2_block_desc_t *) buf) + (bg_num % bgd_per_block);
    memcpy(ret_bgd, bgd_entry, sizeof(ext2_block_desc_t));

    return bg_num;
}

uint32_t fs_get_inode(uint32_t inode_num, ext2_inode_t *inode) {
    uint8_t buf[1024];

    // check to see if inode exists on the file system
    if (inode_num > sb.s_inodes_count || inode_num == 0) {
        return -1;
    }

    ext2_block_desc_t bgd;
    fs_load_bgd(inode_num, &bgd);

    // get the correct part of the inode table
    read_block(bgd.bg_inode_table + (inode_num - 1) / inodes_per_block, buf);
    // get the correct inode
    memcpy(inode, ((ext2_inode_t *) buf) + (inode_num - 1) % inodes_per_block,
        sizeof(ext2_inode_t));

    return inode;
}

uint32_t fs_set_inode(uint32_t inode_num, const ext2_inode_t *inode) {
    uint8_t buf[1024];
    // check to see if inode exists on the file system
    if (inode_num > sb.s_inodes_count || inode_num == 0) {
        return;
    }
    
    ext2_block_desc_t bgd;
    fs_load_bgd(inode_num, &bgd);

    // get the correct part of the inode table
    read_block(bgd.bg_inode_table + (inode_num - 1) / inodes_per_block, buf);
    // get the correct inode
    memcpy(((ext2_inode_t *) buf) + (inode_num - 1) % inodes_per_block, inode,
        sizeof(ext2_inode_t));
    write_block(bgd.bg_inode_table + (inode_num - 1) / inodes_per_block, buf);
}

int32_t fs_find_free(uint32_t block_id) {
    uint8_t buf[1024];
    read_block(block_id, buf);
    // find the first free inode in this block group
    for (size_t i = 0; i < 8 * block_size; i++) {
        // isolate a bit at a time and find first free inode
        for (size_t b = 0; b < 8; b++) {
            uint8_t bit_msk = (1 << (b % 8));
            // if found, set the bit in the bitmap
            // and return index
            if ((buf[i] & bit_msk) == 0) {
                buf[i] |= bit_msk;
                write_block(block_id, buf);
                return i * 8 + b;
            }
        }
    }
    // nothing free found in this block, return failure
    return -1;
}

uint32_t fs_get_block_id(uint32_t block_index, const ext2_inode_t *inode) {
    uint32_t buf[256];
    
    // direct block pointers
    if (block_index < 12) {
        return inode->i_block[block_index];
    }
    // block_index now starts at 0 for first singly indirect pointer
    block_index -= 12;

    uint32_t ids_per_block = block_size / sizeof(uint32_t);
    // single indirect block pointers
    if (block_index < ids_per_block) {
        read_block(inode->i_block[12], (uint8_t *) buf);
        return buf[block_index];
    }
    block_index -= ids_per_block;

    // squared
    uint32_t ids_per_block_2 = ids_per_block * ids_per_block;
    // doubly indirect block pointers
    if (block_index < ids_per_block_2) {
        read_block(inode->i_block[13], (uint8_t *) buf);
        read_block(buf[block_index / ids_per_block], (uint8_t *) buf);
        return buf[block_index % ids_per_block];
    }
    block_index -= ids_per_block_2;

    // cubed
    uint32_t ids_per_block_3 = ids_per_block_2 * ids_per_block;
    // triply indirect block pointers
    if (block_index < ids_per_block_3) {
        read_block(inode->i_block[14], (uint8_t *) buf);
        read_block(buf[block_index / ids_per_block_2], (uint8_t *) buf);
        read_block(buf[(block_index % ids_per_block_2) / ids_per_block],
            (uint8_t *) buf);
        return buf[block_index % ids_per_block];
    }

    // 0 is not a valid block to get from an inode block_list
    // indicates the block_index was out of range
    return 0;
}

int fs_set_block_id(uint32_t block_id, uint32_t block_index, ext2_inode_t *inode) {
    uint32_t buf[256];
    
    // direct block pointers
    if (block_index < 12) {
        inode->i_block[block_index] = block_id;
        return 0;
    }
    // block_index now starts at 0 for first singly indirect pointer
    block_index -= 12;

    uint32_t ids_per_block = block_size / sizeof(uint32_t);
    // single indirect block pointers
    if (block_index < ids_per_block) {
        read_block(inode->i_block[12], (uint8_t *) buf);
        buf[block_index] = block_id;
        write_block(inode->i_block[12], (uint8_t *) buf);
        return 0;
    }
    block_index -= ids_per_block;

    // squared
    uint32_t ids_per_block_2 = ids_per_block * ids_per_block;
    // doubly indirect block pointers
    if (block_index < ids_per_block_2) {
        read_block(inode->i_block[13], (uint8_t *) buf);
        read_block(buf[block_index / ids_per_block], (uint8_t *) buf);
        buf[block_index % ids_per_block] = block_id;
        write_block(buf[block_index / ids_per_block], (uint8_t *) buf);
        return 0;
    }
    block_index -= ids_per_block_2;

    // cubed
    uint32_t ids_per_block_3 = ids_per_block_2 * ids_per_block;
    // triply indirect block pointers
    if (block_index < ids_per_block_3) {
        read_block(inode->i_block[14], (uint8_t *) buf);
        read_block(buf[block_index / ids_per_block_2], (uint8_t *) buf);
        read_block(buf[(block_index % ids_per_block_2) / ids_per_block],
            (uint8_t *) buf);
        buf[block_index % ids_per_block] = block_id;
        write_block(buf[(block_index % ids_per_block_2) / ids_per_block],
            (uint8_t *) buf);
        return 0;
    }

    return -1;
}

void fs_setup_blank_dir(uint32_t blank_num, const ext2_inode_t *blank_inode,
    uint32_t parent_num) {

    uint8_t buf[1024];
    ext2_dir_entry_t dir_entry;
    uint8_t name[2] = {'.', '.'};

    dir_entry.inode = blank_num;
    dir_entry.name_len = 1;
    dir_entry.rec_len =
        sizeof(ext2_dir_entry_t) + ROUND_UP(dir_entry.name_len, 4);
    uint32_t tot_entry_size = dir_entry.rec_len;

    // set . directory
    memcpy(buf, &dir_entry, sizeof(ext2_dir_entry_t));
    memcpy(buf + sizeof(ext2_dir_entry_t), &name, 1);

    // set .. directory
    dir_entry.inode = parent_num;
    dir_entry.name_len = 2;
    memcpy(buf + tot_entry_size, &dir_entry, 8);
    memcpy(buf + tot_entry_size + sizeof(ext2_dir_entry_t), &name, 2);

    // set sentinal directory with inode = 0 and rec_len rest of block
    dir_entry.inode = 0;
    dir_entry.name_len = 0;
    dir_entry.rec_len = block_size - tot_entry_size * 2;
    memcpy(buf + tot_entry_size * 2, &dir_entry, sizeof(ext2_dir_entry_t));

    write_block(blank_inode->i_block[0], buf);
}

void fs_ls(const ext2_inode_t *dir_inode) {
    uint8_t buf[1024];

    if ((dir_inode->i_mode & 0xF000) != EXT2_S_IFDIR) {
        // error this is not a directory
        return;
    }

    read_block(dir_inode->i_block[0], buf);
    size_t i = 0;
    while (1) {
        // first entry will be at start of this block
        ext2_dir_entry_t *entry = (ext2_dir_entry_t *) (buf + i);
        if (entry->inode == 0) {
            break;
        }

        printf("%.*s ", entry->name_len, buf + i + sizeof(ext2_dir_entry_t));
        i += entry->rec_len;
    }

    printf("\n");

}

// try to find the specified file in the specified directory
// returns the inode number and inode struct, or 0 and NULL if not found
uint32_t fs_get_file(const char *name, const ext2_inode_t *dir_inode,
    ext2_inode_t *ret_inode) {

    uint8_t buf[1024];

    if ((dir_inode->i_mode & EXT2_TYPE_MASK) != EXT2_S_IFDIR) {
        // error this is not a directory
        return 0;
    }

    read_block(dir_inode->i_block[0], buf);
    size_t i = 0;
    size_t b = 0;
    while (1) {
        ext2_dir_entry_t *entry = (ext2_dir_entry_t *) (buf + i);
        // check to see if name matches
        if (entry->name_len == strlen(name) &&
                memcmp(entry + 1, name, entry->name_len) == 0) {
            
            ext2_inode_t file_inode;
            fs_get_inode(entry->inode, &file_inode);
            memcpy(ret_inode, &file_inode, sizeof(ext2_inode_t));
            return entry->inode;
        // else go to next entry
        } else {
            i += entry->rec_len;
            // check if we need to go into next block
            if (i >= block_size) {
                i -= block_size;
                b++;
                uint32_t next_block = fs_get_block_id(b, dir_inode);
                // if next block is unused, this is the marker for last entry
                if (next_block == 0) {
                    ret_inode = NULL;
                    return 0;
                } else {
                    read_block(next_block, buf);
                }
            }
        }
        
    }
}

// takes an absolute path and returns the inode number of the file
// or 0 and NULL if it does not exist
uint32_t fs_path_to_inode(const char *path, ext2_inode_t *ret_inode) {

    // make copy of path as strtok modifies it
    char *path_cpy = kmalloc(strlen(path));
    if (path_cpy == NULL) {
        while(1);
    }
    memcpy(path_cpy, path, strlen(path) + 1);

    // set the initial value to the root inode
    ext2_inode_t inode;
    uint32_t inode_num = fs_get_inode(EXT2_ROOT_INO, &inode);
    if (inode_num == 0) {
        while(1);
    }
    // setup return vals in case it is the root inode
    inode_num = EXT2_ROOT_INO;
    *ret_inode = inode;
    
    // get the initial token
    char *sub_dir = strtok(path_cpy, "/");
    // iterate through the whole path
    while (sub_dir != NULL) {
        inode_num = fs_get_file(sub_dir, &inode, &inode);
        // if the specified sub dir was not found, this dir doesn't exist
        if (inode_num == 0) {
            kfree(path_cpy);
            ret_inode = NULL;
            return 0;
        }
        sub_dir = strtok(NULL, "/");
    }
    // if we made it through the loop, this last element in the desired file
    kfree(path_cpy);
    *ret_inode = inode;
    return inode_num;
}

// make a regular file or directory at the specified directory
int fs_mkfile(const char *name, uint32_t dir_num, const ext2_inode_t *dir_inode,
    uint16_t i_mode, uint16_t uid, uint16_t gid) {

    uint8_t buf[1024];

    // these two are reused later if the check succeeds
    ext2_inode_t file_inode;
    uint32_t file_num = fs_get_file(name, dir_inode, &file_inode);
    // error if file exists
    if (file_num != 0) {
        return -1;
    }

    // 1. setup the new inode

    // load inode bitmap block for given directory inode
    ext2_block_desc_t bgd;
    uint32_t bgd_num = fs_load_bgd(dir_num, &bgd);
    // TODO, since inode bitmap does not use full page
    // current code will break when it gets past number per group
    // need to set the remainder to used or some other solution
    // get the first free block of this block group
    int32_t first_free = fs_find_free(bgd.bg_inode_bitmap);
    // for now fail if cannot find one in this group
    if (first_free == -1) {
        return -1;
    }
    // need to adjust if this was not in the first block group
    // also add one because inodes start at one
    file_num = first_free + bgd_num * sb.s_inodes_per_group + 1;
    
    memset(&file_inode, 0, sizeof(ext2_inode_t));
    file_inode.i_mode = i_mode;
    file_inode.i_uid = uid;
    file_inode.i_gid = gid;
    file_inode.i_blocks = 1;
    // rest of members are not dealt with for now

    // 2. allocate it a block

    // for now this fails if the first block is empty
    first_free = fs_find_free(bgd.bg_block_bitmap);
    if (first_free == -1) {
        return -1;
    }
    file_inode.i_block[0] = sb.s_first_data_block + bgd_num * sb.s_blocks_per_group + first_free;
    // specialized setup for each file type
    if ((file_inode.i_mode & 0xF000) == EXT2_S_IFREG) {
        file_inode.i_links_count = 1;
    } else if ((file_inode.i_mode & 0xF000) == EXT2_S_IFDIR) {
        file_inode.i_links_count = 2;
        fs_setup_blank_dir(file_num, &file_inode, dir_num);
    }

    // 3. write it to disk

    fs_set_inode(file_num, &file_inode);


    // 4. add this file to the specified directory
    // TODO add check to ensure this can actually fit within dir file

    ext2_dir_entry_t *last_entry;
    size_t i = 0, b = 0;
    read_block(dir_inode->i_block[0], buf);
    
    while (1) {
        last_entry = (ext2_dir_entry_t *) (buf + i);

        i += last_entry->rec_len;
        // check if we need to go into next block
        if (i >= block_size) {
            i -= block_size;
            b++;
            uint32_t next_block = fs_get_block_id(b, dir_inode);
            // if next block is unused, this is the marker for last entry
            if (next_block == 0) {
                break;
            } else {
                read_block(next_block, buf);
            }
        }        
    }

    // last entry now points to the last directory entry
    // now need to use its free space to create the new entry

    // first check if we'll have enough space to make the new entry in this block
    uint32_t new_last_min_len =
        sizeof(ext2_dir_entry_t) + ROUND_UP(strlen(name), 4);

    // figure out how much space the old last entry needs
    uint32_t old_last_rec_len =
        sizeof(ext2_dir_entry_t) + ROUND_UP(last_entry->name_len, 4);

    uint32_t new_empty_space = last_entry->rec_len - old_last_rec_len;

    if (new_last_min_len > new_empty_space) {
        // need to go into next block
        while(1);
    }
    
    // shorten old last entry to smallest required amount
    last_entry->rec_len = old_last_rec_len;

    // setup new last entry
    last_entry = (ext2_dir_entry_t *) ((uint8_t *) (last_entry) + old_last_rec_len);

    last_entry->inode = file_num;
    last_entry->rec_len = new_empty_space;
    last_entry->name_len = strlen(name);
    last_entry->file_type = 1;
    memcpy(last_entry + 1, name, last_entry->name_len);
    write_block(fs_get_block_id(b - 1, dir_inode), buf);


    return 0;

}

int fs_rmfile(const char *name, uint32_t dir_num, const ext2_inode_t *dir_inode) {
    while(1);
}

int fs_readfile(const char *path, uint32_t start, uint32_t num, uint8_t *buf) {
    
    ext2_inode_t inode;
    uint32_t inum = fs_path_to_inode(path, &inode);

    if (inum == 0) {
        return -1;
    }

    uint32_t blocks_read = MIN(num, inode.i_blocks);
    for (size_t i = start; i < start + blocks_read; i++) {
        read_block(fs_get_block_id(i, &inode), buf + i * block_size);
    }

    return blocks_read;
}

int fs_writefile(const char *path, uint32_t start, uint32_t num, uint8_t *buf) {

    ext2_inode_t inode;
    uint32_t inum = fs_path_to_inode(path, &inode);

    if (inum == 0) {
        return -1;
    }

    for (size_t i = start; i < start + num; i++) {
        uint32_t block_id;
        // if index equals block count, block has yet to be assigned
        if (i == inode.i_blocks) {
            ext2_block_desc_t bgd;
            uint32_t bgd_num = fs_load_bgd(inum, &bgd);
            int32_t first_free = fs_find_free(bgd.bg_block_bitmap);
            // for now fail if cannot find one in this group
            if (first_free == -1) {
                return -1;
            }
            block_id = sb.s_first_data_block + bgd_num * sb.s_blocks_per_group + first_free;;
            fs_set_block_id(block_id, i, &inode);
            inode.i_blocks++;

        // there is already a block for this index
        } else {
            block_id = fs_get_block_id(i, &inode);
        }

        write_block(block_id, buf + (i - start) * block_size);
    }
    // TODO testing, no error checks here
    inode.i_size += num;
    fs_set_inode(inum, &inode);

    return 0;    
}