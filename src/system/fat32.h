#ifndef INCLUDE_FAT32
#define INCLUDE_FAT32

/**
 * Basic FAT32 driver that assumes a MBR formatted disk
 * and only reads from the first partition
 * 
 * TODO:
 * - VFS is not supported
 * - not reentrant
 * - ability to handle failure from hardware functions
 * - more descriptive failure than just -1
 * - testing with sector sizes above 512 bytes
 * - make use of free file records created by deleting files when making new ones
 * currently it always just goes to the end of the directory and adds a new entry there
 * - high 4 reserved FAT bits are overwritten on modifications
 * - directory size is not accurately kept track of
 * 
 * References:
 * https://www.pjrc.com/tech/8051/ide/fat32.html
 * http://read.pudn.com/downloads77/ebook/294884/FAT32%20Spec%20%28SDA%20Contribution%29.pdf
 */

// currently testing using files within linux
// thus using the system clib not the OSes
#include <stdbool.h>
#include "clib/string.h"
#include "utils/macros.h"

#define SECTOR_SIZE 512

// first partition for MBR
#define FIRST_PART_OFFSET 446

#define PATH_DELIM_STR "/"
#define PATH_DELIM_CHR '/'

#define CLUSTER_MASK 0x0FFFFFFF
#define CLUSTER_LAST 0x0FFFFFFF

// NOTE: only the dir attribute is currently used 
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
    char name[11];
    uint8_t attrib;
    uint8_t UNUSED1[8];
    uint16_t cluster_high;
    uint8_t UNUSED2[4];
    uint16_t cluster_low;
    uint32_t size;
} __attribute__((packed)) fat32_record_t;

/* private */
typedef struct {
    // number of cluster that stores the record
    uint32_t cluster;
    // which sector within that cluster (i.e. 0, 1, 2, ...)
    uint32_t sector;
    // treating that sector as an array of records, the index 
    uint32_t index;
} fat32_record_loc_t;

// how many custer numbers fit within each sector in the FAT
#define CLUSTER_NUMS_PER_SECTOR (SECTOR_SIZE / sizeof(uint32_t))
#define RECORDS_PER_SECTOR (SECTOR_SIZE / sizeof(fat32_record_t))


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

// grabs the high and low parts of the cluster and combines them
uint32_t make_cluster(const fat32_record_t *record);

/**
 * Gets a logical block address for use with the underlying block storage device,
 * must be a device with blocks/sectors of 512 bytes
 * @param cluster the cluster of which we want the first sector
 * @return the lba of the desired sector
 */
uint32_t cluster_to_lba(uint32_t cluster);

/**
 * Gets the first free cluster in the FAT
 * @return the number of the first free cluster
 * or 0 if none found (since clusters 0 and 1 are invalid)
 */
uint32_t get_free_cluster();

/**
 * Follows the cluster chain in the FAT
 * @param cluster the known cluster
 * @return the next cluster in the chain, or the end of cluster marker
 */
uint32_t get_next_cluster(uint32_t cluster);

/**
 * Follows the cluster chain in the FAT, but will allocated new clusters
 * if it hits the end of a chain
 * @param cluster the known cluster
 * @return the next cluster in the chain
 */
uint32_t get_or_make_next_cluster(uint32_t cluster);

/**
 * Sets a cluster in the FAT to a value
 * @param index the index in the FAT you want to change
 * @param num the cluster number to set at the desired location
 * @return 0 if successful, -1 if failed
 */
int set_cluster(uint32_t index, uint32_t num);

/**
 * Check whether a given file/dir string matches the stored 8.3 one
 * @param file_str the name as a string, i.e. "TEST.TXT"
 * @param file_83 pointer to the 11 char array that stores the 8.3 file name
 * @return true if the names match, false otherwise
 */
bool names_match(const char *file_str, const char *file_83);

/**
 * Convert a name that's part of a normal string into an 8.3 format
 * @param file_str the name as a string, i.e. "TEST.TXT"
 * @param file_83 pointer to the 11 char array that stores the 8.3 file name
 * @return 0 on success, -1 if invalid name
 */
int name_str_to_83(const char *file_str, char *file_83);

/**
 * Finds a file given a name and the first cluster that stores the directory it's in
 * @param name the name of the file
 * @param dir_cluster the number of the first cluster that stores the directory
 * you want to search in
 * @param ret_rec pointer to a directory record so it can returned if found
 * @param ret_loc pointer to a location struct so it can easily be modified later if necessary
 * @return 0 on success, -1 on failure if file was not found
 * if file not found, the return structs will reference the end of directory record
 */
int get_record_in_dir(const char *name, uint32_t dir_cluster, fat32_record_t *ret_rec, fat32_record_loc_t *ret_loc);

/**
 * Finds a file given its absolute path
 * @param path the absolute path to the file
 * @param ret_rec pointer to a directory record so it can returned if found
 * @param ret_loc pointer to a location struct so it can easily be modified later if necessary
 * @return 0 on success, -1 on failure if file was not found
 */
int get_record(char *path, fat32_record_t *ret_rec, fat32_record_loc_t *ret_loc);

/**
 * Sets a directory record on the storage device
 * @param rec a pointer to the new value for the record
 * @param rec_loc the location of the record
 * @return 0 on success, -1 on failure if file was not found
 */
int set_record(const fat32_record_t *rec, const fat32_record_loc_t *rec_loc);

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
int fat32_writefile(char *path, uint32_t start, uint32_t num, uint8_t *buf);

/**
 * Makes a new file at the given path
 * The preceeding directories must already exist
 * @param path the absolute path of the new file
 * @param is_dir true for directory, false for file
 * @return 0 for success, -1 for failure
 */
int fat32_makefile(char *path, bool is_dir);

/**
 * Deletes a file or directory (if empty) given a path
 * @param path the absolute path of the new file
 * @return 0 for success, -1 for failure
 */
int fat32_deletefile(char *path);

#endif
