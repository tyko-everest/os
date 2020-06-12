#include "file_system.h"

static fs_super_block_t sb;
// to convert block num to sector num
// default to 1 KiB block sizes, also necessary to read in super block
// with read_block function
static uint32_t spb = 2;
static uint32_t block_size = 1024;
// number of block group descriptors that can fit in one block
static uint32_t bgd_per_block;
static uint32_t inodes_per_block;

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

uint32_t fs_get_block_id(uint32_t block_index, const fs_inode_t *inode) {
    uint32_t buf[256];
    
    // direct block pointers
    if (block_index < 12) {
        return inode->block_list[block_index];
    }
    // block_index now starts at 0 for first singly indirect pointer
    block_index -= 12;

    uint32_t ids_per_block = block_size / sizeof(uint32_t);
    // single indirect block pointers
    if (block_index < ids_per_block) {
        read_block(inode->block_list[12], (uint8_t *) buf);
        return buf[block_index];
    }
    block_index -= ids_per_block;

    // squared
    uint32_t ids_per_block_2 = ids_per_block * ids_per_block;
    // doubly indirect block pointers
    if (block_index < ids_per_block_2) {
        read_block(inode->block_list[13], (uint8_t *) buf);
        read_block(buf[block_index / ids_per_block], (uint8_t *) buf);
        return buf[block_index % ids_per_block];
    }
    block_index -= ids_per_block_2;

    // cubed
    uint32_t ids_per_block_3 = ids_per_block_2 * ids_per_block;
    // triply indirect block pointers
    if (block_index < ids_per_block_3) {
        read_block(inode->block_list[14], (uint8_t *) buf);
        read_block(buf[block_index / ids_per_block_2], (uint8_t *) buf);
        read_block(buf[(block_index % ids_per_block_2) / ids_per_block],
            (uint8_t *) buf);
        return buf[block_index % ids_per_block];
    }

    // 0 is not a valid block to get from an inode block_list
    // indicates the block_index was out of range
    return 0;
}

int fs_set_block_id(uint32_t block_id, uint32_t block_index, fs_inode_t *inode) {
    uint32_t buf[256];
    
    // direct block pointers
    if (block_index < 12) {
        inode->block_list[block_index] = block_id;
        return 0;
    }
    // block_index now starts at 0 for first singly indirect pointer
    block_index -= 12;

    uint32_t ids_per_block = block_size / sizeof(uint32_t);
    // single indirect block pointers
    if (block_index < ids_per_block) {
        read_block(inode->block_list[12], (uint8_t *) buf);
        buf[block_index] = block_id;
        write_block(inode->block_list[12], (uint8_t *) buf);
        return 0;
    }
    block_index -= ids_per_block;

    // squared
    uint32_t ids_per_block_2 = ids_per_block * ids_per_block;
    // doubly indirect block pointers
    if (block_index < ids_per_block_2) {
        read_block(inode->block_list[13], (uint8_t *) buf);
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
        read_block(inode->block_list[14], (uint8_t *) buf);
        read_block(buf[block_index / ids_per_block_2], (uint8_t *) buf);
        read_block(buf[(block_index % ids_per_block_2) / ids_per_block],
            (uint8_t *) buf);
        buf[block_index % ids_per_block] = block_id;
        write_block(buf[(block_index % ids_per_block_2) / ids_per_block],
            (uint8_t *) buf);
        return 0;
    }

    // 0 is not a valid block to get from an inode block_list
    // indicates the block_index was out of range
    return 0;
}

void fs_ls(const fs_inode_t *dir_inode) {
    uint8_t buf[1024];

    if ((dir_inode->mode & 0xF000) != UXT_FDIR) {
        // error this is not a directory
        return;
    }

    read_block(dir_inode->block_list[0], buf);
    size_t i = 0;
    while (1) {
        // first entry will be at start of this block
        fs_dir_entry_t *entry = (fs_dir_entry_t *) (buf + i);
        if (entry->inode == 0) {
            break;
        }

        printf("%.*s ", entry->name_len, buf + i + sizeof(fs_dir_entry_t));
        i += entry->offset;
    }

    printf("\n");

}

// try to find the specified file in the specified directory
uint32_t fs_get_file(const char *name, const fs_inode_t *dir_inode,
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
        inode_num = fs_get_file(sub_dir, &dir_inode, &dir_inode);
        // if the specified sub dir was not found, this dir doesn't exist
        if (inode_num == 0) {
            return 0;
        }
        sub_dir = strtok(NULL, "/");
    }
    return inode_num;
}

// make a regular file at the specified directory
int fs_mkfile(const char *name, uint32_t dir_num, const fs_inode_t *dir_inode,
    uint16_t mode, uint16_t uid, uint16_t gid) {

    uint8_t buf[1024];

    // these two are reused later if the check succeeds
    fs_inode_t file_inode;
    uint32_t file_num = fs_get_file(name, dir_inode, &file_inode);
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
    file_inode.user = uid;
    file_inode.group = gid;
    file_inode.blocks_count = 1;
    // rest of members are not dealt with for now

    // 2. allocate it a block

    // for now this fails if the first block is empty
    first_free = fs_find_free(bgd.block_bitmap);
    if (first_free == -1) {
        return -1;
    }
    file_inode.block_list[0] = first_free + bgd.first_block;
    // specialized setup for each file type
    if ((file_inode.mode & 0xF000) == UXT_FREG) {
        file_inode.links = 1;
    } else if ((file_inode.mode & 0xF000) == UXT_FDIR) {
        file_inode.links = 2;
        fs_setup_blank_dir(file_num, &file_inode, dir_num);
    }

    // 3. write it to disk

    fs_set_inode(file_num, &file_inode);


    // 4. add this file to the specified directory
    // TODO add check to ensure this can actually fit within dir file

    read_block(dir_inode->block_list[0], buf);
    size_t i = 0;
    fs_dir_entry_t *last_entry;
    do {
        // first entry will be at start of this block
        last_entry = (fs_dir_entry_t *) (buf + i);
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
    last_entry = (fs_dir_entry_t *) (buf + block_size - new_last_offset);
    last_entry->inode = 0;
    last_entry->offset = new_last_offset;
    write_block(dir_inode->block_list[0], buf);


    return 0;

}

int fs_rmfile(const char *name, uint32_t dir_num, const fs_inode_t *dir_inode) {

}

int fs_readfile(const char *name, uint32_t dir_num, const fs_inode_t *dir_inode,
    uint32_t start, uint32_t num, uint8_t *buf) {
    
    fs_inode_t inode;
    uint32_t inum = fs_get_file(name, dir_inode, &inode);

    if (inum == 0) {
        return -1;
    }

    uint32_t blocks_read = MIN(num, inode.blocks_count);
    for (size_t i = start; i < start + blocks_read; i++) {
        read_block(fs_get_block_id(i, &inode), buf + i * block_size);
    }

    return blocks_read;
}

int fs_writefile(const char *name, uint32_t dir_num, const fs_inode_t *dir_inode,
    uint32_t start, uint32_t num, uint8_t *buf) {

    fs_inode_t inode;
    uint32_t inum = fs_get_file(name, dir_inode, &inode);

    if (inum == 0) {
        return -1;
    }

    for (size_t i = start; i < start + num; i++) {
        uint32_t block_id;
        // if index equals block count, block has yet to be assigned
        if (i == inode.blocks_count) {
            fs_block_desc_t bgd;
            uint32_t bgd_num = fs_load_bgd(dir_num, &bgd);
            int32_t first_free = fs_find_free(bgd.block_bitmap);
            // for now fail if cannot find one in this group
            if (first_free == -1) {
                return -1;
            }
            block_id = first_free + bgd.first_block;
            fs_set_block_id(block_id, i, &inode);
            inode.blocks_count++;

        // there is already a block for this index
        } else {
            block_id = fs_get_block_id(i, &inode);
        }

        write_block(block_id, buf + (i - start) * block_size);
    }

    fs_set_inode(inum, &inode);

    return 0;    
}