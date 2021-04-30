#ifndef INCLUDE_FAT32
#define INCLUDE_FAT32

/**
 * Basic FAT32 driver that assumes a MBR formatted disk
 * and only reads from the first partition
 * VFS is not supported
 * 
 * TODO:
 * - ability to handle failure from hardware functions
 * - more descriptive failure than just -1
 * - make functions for: writing, making + deleting files and dirs
 * 
 * References:
 * https://www.pjrc.com/tech/8051/ide/fat32.html
 * http://read.pudn.com/downloads77/ebook/294884/FAT32%20Spec%20%28SDA%20Contribution%29.pdf
 */

// currently testing using files within linux
// thus using the system clib not the OSes
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../clib/stdint.h"
#include "../utils/macros.h"

#define SECTOR_SIZE 512

// first partition for MBR
#define FIRST_PART_OFFSET 446

#define PATH_DELIM "/"

#define FAT_ATTR_RDONLY     (1 << 0)
#define FAT_ATTR_HIDDEN     (1 << 1)
#define FAT_ATTR_SYS        (1 << 2)
#define FAT_ATTR_VOLID      (1 << 3)
#define FAT_ATTR_DIR        (1 << 4)
#define FAT_ATTR_ARCHIVE    (1 << 5)   

typedef struct {
    uint8_t boot_flag;
    uint8_t chs_begin[3];
    uint8_t type_code;
    uint8_t chs_end[3];
    uint32_t lba_begin;
    uint32_t num_sectors;
} __attribute__((packed)) mbr_partition_entry_t;

typedef struct {
    uint8_t jmp_boot[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sec;
    uint8_t secs_per_cluster;
    uint16_t reserved_sec_count;
    uint8_t num_fats;
    uint8_t UNUSED1[19];
    uint32_t secs_per_fat;
    uint16_t ext_flags;
    uint16_t fs_ver;
    uint32_t root_cluster;
    uint8_t UNUSED2[462];
    uint16_t signature;
} __attribute__((packed)) fat32_vol_id_t;

typedef struct {
    uint8_t name[11];
    uint8_t attrib;
    uint8_t UNUSED1[8];
    uint16_t cluster_high;
    uint8_t UNUSED2[4];
    uint16_t cluster_low;
    uint32_t size;
} __attribute__((packed)) fat32_dir_t;


/* functions that interface with whatever actually gets the bytes
 * i.e. the ide driver, stdio for testing, sd card eventually
 */

// these currently use stdio to read from an image file the represents the disk
// this setup must be called before anything else
void setup_sector();
// read or write a sector at a time given its lba
void read_sector(uint32_t sector_lba, uint8_t *buf);
void write_sector(uint32_t sector_lba, uint8_t *buf);


/* internal helper function */

/**
 * Gets a logical block address for use with the underlying block storage device,
 * must be a device with blocks/sectors of 512 bytes
 * @param cluster the cluster of which we want the first sector
 * @return the lba of the desired sector
 */
uint32_t cluster_to_lba(uint32_t cluster);

/**
 * Follows the cluster chain in the FAT
 * @param cluster the known cluster
 * @return the next cluster in the chain
 */
uint32_t get_next_cluster(uint32_t cluster);

/**
 * Check whether a given file/dir string matches the stored 8.3 one
 * @param name the name as a string, i.e. "TEST.TXT"
 * @param stored pointer to the 11 char array that stores the 8.3 file name
 * @return true if the names match, false otherwise
 */
bool names_match(const char *file_str, const char *file_83);

/**
 * Finds a file given a name and the first cluster that stores the directory it's in
 * @param name the name of the file
 * @param dir_cluster the number of the first cluster that stores the directory
 * you want to search in
 * @param ret_dir pointer to a directory entry so it can returned if found
 * @return 0 on success, -1 on failure if file was not found
 */
int name_to_dir_entry(const char *name, uint32_t dir_cluster, fat32_dir_t *ret_dir);

/**
 * Finds a file given its absolute path
 * @param path the absolute path to the file
 * @param ret_dir pointer to a directory entry so it can returned if found
 * @return 0 on success, -1 on failure if file was not found
 */
int path_to_dir_entry(char *path, fat32_dir_t *ret_dir);


/* external functions meant to be used with the rest of the os */

/**
 * Initializes some static parameters used for calculations by the driver
 * Must be called before any other functions are used
 */
void fat32_init();

/**
 * Reads a number of bytes from a file
 * @param path the absolute path to the file
 * @param start the byte offset into the file where the reading will start
 * @param num the number of bytes to read
 * @param buf a buffer to read into
 * @return the number of bytes read, can be less than requested if EOF hit
 * or -1 on an error
 */
int fat32_readfile(char *path, uint32_t start, uint32_t num, uint8_t *buf);

/**
 * Writes a number of bytes to a file
 * @param path the absolute path to the file
 * @param start the byte offset into the file where the writing will start
 * @param num the number of bytes to write
 * @param buf a buffer to write from
 * @return the number of bytes written, can be less than requested,
 * or -1 on an error
 */
int fat32_writefile(const char *path, uint32_t start, uint32_t num, uint8_t *buf);

#endif
