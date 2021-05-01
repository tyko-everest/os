#include "fat32.h"

static uint8_t read_buf[SECTOR_SIZE];
static FILE *fd;

static uint32_t fat_begin_lba;
static uint32_t cluster_begin_lba;
static uint32_t sectors_per_cluster;
static uint32_t root_dir_first_cluster;
static uint32_t secs_per_fat;


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

uint32_t get_free_cluster() {
    uint32_t buf[CLUSTER_NUMS_PER_SECTOR];
    for (int sec = 0; sec < secs_per_fat; sec++) {
        read_sector(fat_begin_lba + sec, (uint8_t*) buf);
        for (int num = 0; num < CLUSTER_NUMS_PER_SECTOR; num++) {
            if (buf[num] == 0) {
                return sec * CLUSTER_NUMS_PER_SECTOR + num;
            }
        }
    }
    return 0;
}

uint32_t get_next_cluster(uint32_t cluster) {
    static uint32_t buf[CLUSTER_NUMS_PER_SECTOR];
    read_sector(fat_begin_lba + cluster / CLUSTER_NUMS_PER_SECTOR, (uint8_t*) buf);
    return buf[cluster % CLUSTER_NUMS_PER_SECTOR];
}

uint32_t get_or_make_next_cluster(uint32_t cluster) {
    uint32_t old_cluster = cluster;
    cluster = get_next_cluster(cluster);
    if ((cluster & CLUSTER_MASK) == CLUSTER_LAST) {
        cluster = get_free_cluster();
        if (cluster == 0) {
            while(1);
        }
        set_cluster(old_cluster, cluster);
        set_cluster(cluster, CLUSTER_LAST);
    }
    return cluster;
}

int set_cluster(uint32_t index, uint32_t num) {
    static uint32_t buf[CLUSTER_NUMS_PER_SECTOR];
    if (index < 2) {
        return -1;
    }
    uint32_t lba = fat_begin_lba + index / CLUSTER_NUMS_PER_SECTOR;
    read_sector(lba, (uint8_t*) buf);
    buf[index % CLUSTER_NUMS_PER_SECTOR] = num;
    write_sector(lba, (uint8_t*) buf);
    return 0;
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
    if (file_str_ext_len == 0) {
        ext_match = memcmp("   ", file_83 + 8, 3) == 0;
    } else {
        ext_match = memcmp(file_str + file_str_name_len + 1, file_83 + 8, file_str_ext_len) == 0;
    }
    
    return ext_match;
}


int get_record_in_dir(const char *name, uint32_t dir_cluster, fat32_record_t *ret_rec, fat32_record_loc_t *ret_loc) {
    static fat32_record_t buf[SECTOR_SIZE / sizeof(fat32_record_t)];

    for (;;) {

        // starting from 0, which sector of the current cluster are we on
        for (int sector_num = 0; sector_num < sectors_per_cluster; sector_num++) {
            uint32_t lba = cluster_to_lba(dir_cluster) + sector_num;
            read_sector(lba, (uint8_t*) buf);

            // in the given sector, which directory are we at
            for (int dir_index = 0; dir_index < SECTOR_SIZE / sizeof(fat32_record_t); dir_index++) {
                // check if end of directory list had been reached
                if (buf[dir_index].name[0] == 0) {
                    return -1;
                }
                if (names_match(name, buf[dir_index].name)) {
                    memcpy(ret_rec, buf + dir_index, sizeof(fat32_record_t));
                    ret_loc->index = dir_index;
                    ret_loc->sector = lba;
                    return 0;
                }
            }
        }

        dir_cluster = get_next_cluster(dir_cluster);
    }
}

int get_record(char *path, fat32_record_t *ret_rec, fat32_record_loc_t *ret_loc) {

    uint32_t dir_cluster = root_dir_first_cluster;
    fat32_record_t rec;
    fat32_record_loc_t loc;

    bool prev_was_file = false;
    char *name = strtok(path, PATH_DELIM);
    // assume path has been checked to not be null
    while (name != NULL) {
        // if a object was found that was a file, but it was not the last part of the path
        // the path must be invalid
        if (prev_was_file) {
            return -1;
        }
        int res = get_record_in_dir(name, dir_cluster, &rec, &loc);
        if (res < 0) {
            return -1;
        } else {
            if (!(rec.attrib & FAT_ATTR_DIR)) {
                prev_was_file = true;
            }
            name = strtok(NULL, PATH_DELIM);
        }
        dir_cluster = (rec.cluster_high << 16) | rec.cluster_low;
    }
    memcpy(ret_rec, &rec, sizeof(fat32_record_t));
    memcpy(ret_loc, &loc, sizeof(fat32_record_loc_t));
    return 0;
}

int set_record(const fat32_record_t *rec, const fat32_record_loc_t *rec_loc) {
    static fat32_record_t buf[SECTOR_SIZE / sizeof(fat32_record_t)];
    read_sector(rec_loc->sector, (uint8_t*) buf);
    memcpy(buf + rec_loc->index, rec, sizeof(fat32_record_t));
    write_sector(rec_loc->sector, (uint8_t*) buf);
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
    secs_per_fat = vol_info.secs_per_fat;
}

int fat32_readfile(char *path, uint32_t start, uint32_t num, uint8_t *buf) {
    uint8_t temp_buf[SECTOR_SIZE];
    uint32_t count = 0;

    fat32_record_t record;
    fat32_record_loc_t record_loc;
    if (get_record(path, &record, &record_loc) < 0) {
        return -1;
    }

    // check if this is a directory
    if (record.attrib & FAT_ATTR_DIR) {
        return -1;
    } 
    
    // check if we are being asked to start beyond the end of the file
    if (start >= record.size) {
        return 0;
    }

    // cap the num so it will not run off the end of the file
    num = MIN(num, record.size - start);

    // these is simply the first cluster in the file
    uint32_t cluster = (record.cluster_high << 16) | record.cluster_low;
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

int fat32_writefile(char *path, uint32_t start, uint32_t num, uint8_t *buf) {
    uint8_t temp_buf[SECTOR_SIZE];
    uint32_t count = 0;

    fat32_record_t record;
    fat32_record_loc_t rec_loc;
    if (get_record(path, &record, &rec_loc) < 0) {
        return -1;
    }

    // check if this is a directory
    if (record.attrib & FAT_ATTR_DIR) {
        return -1;
    } 
    
    // this is simply the first cluster in the file
    uint32_t cluster = (record.cluster_high << 16) | record.cluster_low;
    // if file is empty it will not have a cluster assigned to it yet
    if (cluster == 0) {
        cluster = get_free_cluster();
        record.cluster_high = cluster >> 16;
        record.cluster_low = cluster & 0xFFFF;
        set_cluster(cluster, CLUSTER_LAST);
    }

    // find the first cluster where we need to start writing
    for (int i = 0; i < start / (sectors_per_cluster * SECTOR_SIZE); i++) {
        cluster = get_or_make_next_cluster(cluster);
    }

    // this keeps tracks of which sector within the current cluster
    uint32_t sector = (start / SECTOR_SIZE) % sectors_per_cluster;

    // deal with writing partway through the first sector
    // this is guarenteed to already be an allocated sector for the file
    // since writing past where the file currently ends is not allowed
    uint32_t left_before = start % SECTOR_SIZE;
    if (left_before) {
        uint32_t lba = cluster_to_lba(cluster) + sector;
        read_sector(lba, temp_buf);

        uint32_t remainder = SECTOR_SIZE - left_before;
        if (num >= remainder) {
            memcpy(temp_buf + left_before, buf, remainder);
            write_sector(lba, temp_buf);
            num -= remainder;
            count += remainder;
        } else {
            memcpy(temp_buf + left_before, buf, num);
            write_sector(lba, temp_buf);
            count += num;

            record.size = MAX(record.size, start + count);
            set_record(&record, &rec_loc);
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
    
    // writing the rest we could run past what is already allocated to the file
    // if so need to allocate more clusters in the FAT and update the files size
    // in its directory entry
    for (;;) {
        for (sector = 0; sector < sectors_per_cluster; sector++) {
            uint32_t lba;
        sector_loop:
            lba = cluster_to_lba(cluster) + sector;
            if (num >= SECTOR_SIZE) {
                write_sector(lba, buf + count);
                num -= SECTOR_SIZE;
                count += SECTOR_SIZE;
            } else {
                read_sector(lba, temp_buf);
                memcpy(temp_buf, buf + count, num);
                write_sector(lba, temp_buf);
                count += num;

                record.size = MAX(record.size, start + count);
                set_record(&record, &rec_loc);
                return count;
            }
        }
        cluster = get_or_make_next_cluster(cluster);
    }
    return -1;
}
