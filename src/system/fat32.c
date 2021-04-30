#include "fat32.h"

static uint8_t read_buf[SECTOR_SIZE];
static FILE *fd;

static uint32_t fat_begin_lba;
static uint32_t cluster_begin_lba;
static uint32_t sectors_per_cluster;
static uint32_t root_dir_first_cluster;

void setup_sector() {
    fd = fopen("/home/tyko/os/zzz.img", "r+");
    // fd = fopen("/mnt/c/Users/Tyko/Downloads/2021-03-04-raspios-buster-armhf-lite.img", "r+");
    if (fd < 0) {
        while(1);
    }
}

void read_sector(uint32_t sector_lba, uint8_t *buf) {
    fseek(fd, sector_lba * SECTOR_SIZE, SEEK_SET);
    fread(buf, 1, SECTOR_SIZE, fd);
}

void write_sector(uint32_t sector_lba, uint8_t *buf) {
    fseek(fd, sector_lba * SECTOR_SIZE, SEEK_SET);
    fwrite(buf, 1, SECTOR_SIZE, fd);
}

uint32_t cluster_to_lba(uint32_t cluster) {
    return cluster_begin_lba + (cluster - 2) * sectors_per_cluster;
}

uint32_t get_next_cluster(uint32_t cluster) {
    static uint32_t buf[SECTOR_SIZE / sizeof(uint32_t)];
    uint32_t cluster_nums_per_sector = SECTOR_SIZE / sizeof(uint32_t);
    read_sector(fat_begin_lba + cluster / cluster_nums_per_sector, buf);
    return buf[cluster % cluster_nums_per_sector];
}

bool names_match(const char *file_str, const char *file_83) {
    uint32_t file_str_len = strlen(file_str);
    uint32_t file_str_name_len = strcspn(file_str, ".");
    uint32_t file_str_ext_len = file_str_len - file_str_name_len;
    if (file_str[file_str_name_len] == '.') {
        file_str_ext_len--;
    }

    char *ptr = memchr(file_83, ' ', 8);
    uint32_t file_83_name_len = ptr == NULL ? 8 : ptr - file_83;
    ptr = memchr(file_83 + 8, ' ', 3);
    uint32_t file_83_ext_len = ptr == NULL ? 3 : ptr - (file_83 + 8);

    if (file_str_name_len != file_83_name_len || file_str_ext_len != file_83_ext_len) {
        return false;
    }

    if (memcmp(file_str, file_83, file_str_name_len) != 0) {
        return false;
    }
    bool ext_match;
    // if condition is true there is no extension
    if (file_str_len == file_str_name_len) {
        ext_match = memcmp("   ", file_83 + 8, 3) == 0;
    } else {
        ext_match = memcmp(file_str + file_str_name_len + 1, file_83 + 8, file_str_ext_len) == 0;
    }
    
    return ext_match;
}

int name_to_dir_entry(const char *name, uint32_t dir_cluster, fat32_dir_t *ret_dir) {
    static uint8_t buf[SECTOR_SIZE];

    // which cluster are we on, of the clusters that store this directory list 
    int cluster_num = dir_cluster;
    for (;;) {

        // starting from 0, which sector of the current cluster are we on
        for (int sector_num = 0; sector_num < sectors_per_cluster; sector_num++) {

            read_sector(cluster_to_lba(cluster_num + sector_num), buf);

            // in the given sector, which directory are we at
            for (int dir_index = 0; dir_index < SECTOR_SIZE / sizeof(fat32_dir_t); dir_index++) {

                // get pointer to current directory structure
                fat32_dir_t *curr_dir = (fat32_dir_t*) (buf + dir_index * sizeof(fat32_dir_t));

                // check if end of directory list had been reached
                if (curr_dir->name[0] == 0) {
                    memset(ret_dir, 0, sizeof(fat32_dir_t));
                    return -1;
                }

                if (names_match(name, curr_dir->name)) {
                    memcpy(ret_dir, curr_dir, sizeof(fat32_dir_t));
                    return 0;
                }
            }
        }

        cluster_num = get_next_cluster(cluster_num);
    }
}

int path_to_dir_entry(char *path, fat32_dir_t *ret_dir) {

    uint32_t dir_cluster = root_dir_first_cluster;
    fat32_dir_t dir;

    bool prev_was_file = false;
    char *name = strtok(path, PATH_DELIM);
    // assume path has been checked to not be null
    while (name != NULL) {
        // if a object was found that was a file, but it was not the last part of the path
        // the path must be invalid
        if (prev_was_file) {
            return -1;
        }
        int res = name_to_dir_entry(name, dir_cluster, &dir);
        if (res < 0) {
            return -1;
        } else {
            if (!(dir.attrib & FAT_ATTR_DIR)) {
                prev_was_file = true;
            }
            name = strtok(NULL, PATH_DELIM);
        }
        dir_cluster = (dir.cluster_high << 16) | dir.cluster_low;
    }
    memcpy(ret_dir, &dir, sizeof(fat32_dir_t));
    return 0;
}

void fat32_init() {
    // find the first partition
    read_sector(0, read_buf);
    mbr_partition_entry_t first_part = *(mbr_partition_entry_t*) (read_buf + FIRST_PART_OFFSET);
    read_sector(first_part.lba_begin, read_buf);
    fat32_vol_id_t vol_info = *(fat32_vol_id_t*) read_buf;

    fat_begin_lba = first_part.lba_begin + vol_info.reserved_sec_count;
    cluster_begin_lba = first_part.lba_begin + vol_info.reserved_sec_count + (vol_info.num_fats * vol_info.secs_per_fat);
    sectors_per_cluster = vol_info.secs_per_cluster;
    root_dir_first_cluster = vol_info.root_cluster;
}

int fat32_readfile(char *path, uint32_t start, uint32_t num, uint8_t *buf) {
    uint8_t temp_buf[SECTOR_SIZE];
    uint32_t count = 0;

    fat32_dir_t dir_entry;
    if (path_to_dir_entry(path, &dir_entry) < 0) {
        return -1;
    }

    // check if this is a directory
    if (dir_entry.attrib & FAT_ATTR_DIR) {
        return -1;
    } 
    
    // check if we are being asked to start beyond the end of the file
    if (start >= dir_entry.size) {
        return 0;
    }

    // cap the num so it will not run off the end of the file
    num = MIN(num, dir_entry.size - start);

    // these is simply the first cluster in the file
    uint32_t cluster = (dir_entry.cluster_high << 16) | dir_entry.cluster_low;
    // this keeps tracks of which sector within the current cluster
    uint32_t sector = 0;

    // find the first cluster where we need to start reading
    for (int i = 0; i < start / (sectors_per_cluster * SECTOR_SIZE); i++) {
        cluster = get_next_cluster(cluster);
    }
    // find the sector within that cluster
    sector = (start / SECTOR_SIZE) % sectors_per_cluster;

    // deal with reading partway through the first sector
    uint32_t left_before = start % SECTOR_SIZE;
    if (left_before) {
        read_sector(cluster_to_lba(cluster) + sector, temp_buf);
        if (num >= SECTOR_SIZE - left_before) {
            memcpy(buf, temp_buf + left_before, SECTOR_SIZE - left_before);
            num -= SECTOR_SIZE - left_before;
            count += SECTOR_SIZE - left_before;
        } else {
            memcpy(buf, temp_buf + left_before, num);
            count += num;
            return count;
        }
        // advance to next sector, and cluster if necessary
        sector++;
        if (sector >= sectors_per_cluster) {
            sector = 0;
            cluster = get_next_cluster(cluster);
        }
    }

    goto sector_loop;
    
    // process the remainng full sectors and potentially a partly full one
    for (;;) {
        for (sector = 0; sector < sectors_per_cluster; sector++) {
        sector_loop:
            read_sector(cluster_to_lba(cluster) + sector, temp_buf);
            if (num >= SECTOR_SIZE) {
                memcpy(buf + count, temp_buf, SECTOR_SIZE);
                num -= SECTOR_SIZE;
                count += SECTOR_SIZE;
            } else {
                memcpy(buf + count, temp_buf, num);
                count += num;
                return count;
            }
        }

        cluster = get_next_cluster(cluster);
    }

    return -1;
}