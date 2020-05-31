#include "file_system.h"

static fs_super_block_t sb;
static uint32_t block_size = 1024;
// to convert block num to sector num
// default to 1 KiB block sizes, also necessary to read in super block
// with read_block function
static uint32_t spb = 2;
// number of block group descriptors that can fit in one block
static uint32_t bgd_per_block;
static uint32_t inodes_per_block;

// TODO make this dynamic at some point
// block buffer to load a block into memory
static uint8_t bb[1024];

#ifndef TEST_FS

void read_block(uint32_t block_id, uint8_t *buf) {
    ata_pio_read(ATA0, ATA_MASTER, block_id * spb, spb, buf);
}

void write_block(uint32_t block_id, uint8_t *buf) {
    ata_pio_write(ATA0, ATA_MASTER, block_id * spb, spb, buf);
}

#else

FILE *fp;

void setup_fp() {
    fp = fopen("../disk.img", "r+");
}

void read_block(uint32_t block_id, uint8_t *buf) {
    fseek(fp, block_id * block_size, SEEK_SET);
    fread(buf, 1, block_size, fp);
}

void write_block(uint32_t block_id, uint8_t *buf) {
    fseek(fp, block_id * block_size, SEEK_SET);
    fwrite(buf, 1, block_size, fp);
}

#endif

void fs_init() {
    read_block(0, (uint8_t *) &sb);
    block_size = 1024 << sb.log_block_size;
    spb = block_size / SECTOR_SIZE;
    bgd_per_block = block_size / sizeof(fs_block_desc_t);
    inodes_per_block = block_size / sizeof(fs_inode_t);
}

uint32_t fs_load_bgd(uint32_t inode_num, fs_block_desc_t *ret_bgd) {
    uint8_t buf[1024];
    // which block of the block group desc table has the needed entry
    uint32_t bgdt_block_id =
        (inode_num - 1) / (bgd_per_block * sb.inodes_per_group); 
    // add one for superblock
    read_block(bgdt_block_id + 1, buf);
    // which entry on the selected block holds the right block group entry
    uint32_t bgd_num = (inode_num - 1) / sb.inodes_per_group;
    memcpy(ret_bgd, ((fs_block_desc_t *) buf) + bgd_num,
        sizeof(fs_block_desc_t));
    return bgd_num;
}

fs_inode_t fs_get_inode(uint32_t inode_num) {
    uint8_t buf[1024];

    fs_inode_t inode;
    // check to see if inode exists on the file system
    if (inode_num > sb.inodes_count || inode_num == 0) {
        memset(&inode, 0, sizeof(fs_inode_t));
        return inode;
    }

    fs_block_desc_t bgd;
    fs_load_bgd(inode_num, &bgd);

    // get the correct part of the inode table
    read_block(bgd.inode_table + (inode_num - 1) / inodes_per_block, buf);
    // get the correct inode
    memcpy(&inode, ((fs_inode_t *) buf) + (inode_num - 1) % inodes_per_block,
        sizeof(fs_inode_t));

    return inode;
}

void fs_set_inode(uint32_t inode_num, const fs_inode_t * inode) {
    uint8_t buf[1024];
    // check to see if inode exists on the file system
    if (inode_num > sb.inodes_count || inode_num == 0) {
        return;
    }
    
    fs_block_desc_t bgd;
    fs_load_bgd(inode_num, &bgd);

    // get the correct part of the inode table
    read_block(bgd.inode_table + (inode_num - 1) / inodes_per_block, buf);
    // get the correct inode
    memcpy(((fs_inode_t *) buf) + (inode_num - 1) % inodes_per_block, inode,
        sizeof(fs_inode_t));
    write_block(bgd.inode_table + (inode_num - 1) / inodes_per_block, buf);
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

void fs_setup_blank_dir(uint32_t blank_num, const fs_inode_t *blank_inode,
    uint32_t parent_num) {

    uint8_t buf[1024];
    fs_dir_entry_t dir_entry;
    uint8_t name[2] = {'.', '.'};

    dir_entry.inode = blank_num;
    dir_entry.name_len = 1;
    dir_entry.offset =
        sizeof(fs_dir_entry_t) + ROUND_UP(dir_entry.name_len, 4);
    uint32_t tot_entry_size = dir_entry.offset;

    // set . directory
    uint32_t index = 0;
    memcpy(buf, &dir_entry, sizeof(fs_dir_entry_t));
    memcpy(buf + sizeof(fs_dir_entry_t), &name, 1);

    // set .. directory
    dir_entry.inode = parent_num;
    dir_entry.name_len = 2;
    memcpy(buf + tot_entry_size, &dir_entry, 8);
    memcpy(buf + tot_entry_size + sizeof(fs_dir_entry_t), &name, 2);

    // set sentinal directory with inode = 0 and offset rest of block
    dir_entry.inode = 0;
    dir_entry.name_len = 0;
    dir_entry.offset = block_size - tot_entry_size * 2;
    memcpy(buf + tot_entry_size * 2, &dir_entry, sizeof(fs_dir_entry_t));

    write_block(blank_inode->block_list[0], buf);
}

void fs_ls(const fs_inode_t *dir_inode) {
    if ((dir_inode->mode & 0xF000) != UXT_FDIR) {
        // error this is not a directory
        return;
    }

    read_block(dir_inode->block_list[0], bb);
    size_t i = 0;
    while (1) {
        // first entry will be at start of this block
        fs_dir_entry_t *entry = (fs_dir_entry_t *) (bb + i);
        if (entry->inode == 0) {
            break;
        }

        printf("%.*s ", entry->name_len, bb + i + sizeof(fs_dir_entry_t));
        i += entry->offset;
    }

    printf("\n");

}

// try to find the specified file in the specified directory
uint32_t fs_find_file(const char *name, const fs_inode_t *dir_inode,
    fs_inode_t *ret_inode) {

    uint8_t buf[1024];

    if ((dir_inode->mode & 0xF000) != UXT_FDIR) {
        // error this is not a directory
        return 0;
    }

    read_block(dir_inode->block_list[0], buf);
    size_t i = 0;
    while (1) {
        fs_dir_entry_t *entry = (fs_dir_entry_t *) (buf + i);
        // is this the last entry
        if (entry->inode == 0) {
            return 0;
        // if not check to see if name matches
        } else if (entry->name_len == strlen(name) &&
                memcmp(entry + 1, name, entry->name_len) == 0) {
            
            fs_inode_t file_inode = fs_get_inode(entry->inode);
            memcpy(ret_inode, &file_inode, sizeof(fs_inode_t));
            return entry->inode;
        // else go to next entry
        } else {
            i += entry->offset;
        }
        
    }
}

// takes an absolute path to a directory and returns its inode
uint32_t fs_path_to_inode(char *dir, fs_inode_t *ret_inode) {
    // set the initial value to the root inode
    fs_inode_t dir_inode = fs_get_inode(UXT_ROOT_INO);
    // setup return vals in case it is the root inode
    uint32_t inode_num = UXT_ROOT_INO;
    *ret_inode = dir_inode;
    
    // get the initial token
    char *sub_dir = strtok(dir, "/");
    // iterate through the whole path
    while (sub_dir != NULL) {
        inode_num = fs_find_file(sub_dir, &dir_inode, &dir_inode);
        // if the specified sub dir was not found, this dir doesn't exist
        if (inode_num == 0) {
            return 0;
        }
        sub_dir = strtok(NULL, "/");
    }
    return inode_num;
}

// make a regular file at the specified directory
int fs_mkfile(const char *name, uint32_t dir_num, fs_inode_t dir_inode,
    uint16_t mode, uint16_t uid, uint16_t gid) {

    // fs_inode_t dir_inode;
    // uint32_t dir_num = fs_path_to_inode(dir, &dir_inode);
    // // error if directory does not exist
    // if (dir_num == 0) {
    //     return -1;
    // }

    // these two are reused later if the check succeeds
    fs_inode_t file_inode;
    uint32_t file_num = fs_find_file(name, &dir_inode, &file_inode);
    // error if file exists
    if (file_num != 0) {
        return -1;
    }

    // 1. setup the new inode

    // load inode bitmap block for given directory inode
    fs_block_desc_t bgd;
    uint32_t bgd_num = fs_load_bgd(dir_num, &bgd);
    // TODO, since inode bitmap does not use full page
    // current code will break when it gets past number per group
    // need to set the remainder to used or some other solution
    int32_t first_free = fs_find_free(bgd.inode_bitmap);
    // for now fail if cannot find one in this group
    if (first_free == -1) {
        return -1;
    }
    // need to adjust if this was not in the first block group
    // also add one because inodes start at one
    file_num = first_free + bgd_num * sb.inodes_per_group + 1;
    
    file_inode.mode = mode;
    // one hardlink for files by default
    file_inode.links = 1;
    file_inode.user = uid;
    file_inode.group = gid;
    file_inode.blocks_count = 1;
    // rest of members are not dealt with for now

    // 2. allocate it a block

    first_free = fs_find_free(bgd.block_bitmap);
    if (first_free == -1) {
        return -1;
    }
    file_inode.block_list[0] = first_free + bgd.first_block;
    // if dir, set it up
    if ((file_inode.mode & 0xF000) == UXT_FDIR) {
        file_inode.links = 2;
        fs_setup_blank_dir(file_num, &file_inode, dir_num);
    }

    // 3. write it to disk

    fs_set_inode(file_num, &file_inode);


    // 4. add this file to the specified directory
    // TODO add check to ensure this can actually fit within dir file

    read_block(dir_inode.block_list[0], bb);
    size_t i = 0;
    fs_dir_entry_t *last_entry;
    do {
        // first entry will be at start of this block
        last_entry = (fs_dir_entry_t *) (bb + i);
        i += last_entry->offset;
    } while (last_entry->inode != 0);

    // turn sentinel entry into new entry
    last_entry->inode = file_num;
    last_entry->name_len = strlen(name);
    uint32_t new_entry_offset =
        sizeof(fs_dir_entry_t) + ROUND_UP(last_entry->name_len, 4);
    // remainder of space in block, needed to setup new sentinel entry
    uint32_t new_last_offset = last_entry->offset - new_entry_offset;
    last_entry->offset = new_entry_offset;
    memcpy(last_entry + 1, name, last_entry->name_len);

    // setup new sentinel entry
    last_entry = (fs_dir_entry_t *) (bb + block_size - new_last_offset);
    last_entry->inode = 0;
    last_entry->offset = new_last_offset;
    write_block(dir_inode.block_list[0], bb);


    return 0;

}

