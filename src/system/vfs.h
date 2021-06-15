#ifndef INCLUDE_VFS
#define INCLUDE_VFS

#include "clib/stdbool.h"
#include "clib/stdint.h"
#include "system/fat32.h"

void vfs_init();

/**
 * Reads a number of bytes from a file
 * @param path the absolute path to the file
 * @param start the byte offset into the file where the reading will start
 * @param num the number of bytes to read
 * @param buf a buffer to read into
 * @return the number of bytes read, can be less than requested if EOF hit
 * or -1 on an error
 */
int readfile(const char *path, uint32_t start, uint32_t num, uint8_t *buf);

/**
 * Writes a number of bytes to a file
 * @param path the absolute path to the file
 * @param start the byte offset into the file where the writing will start
 * @param num the number of bytes to write
 * @param buf a buffer to write from
 * @return the number of bytes written, can be less than requested,
 * or -1 on an error
 */
int writefile(const char *path, uint32_t start, uint32_t num, uint8_t *buf);

/**
 * Makes a new file at the given path
 * The preceeding directories must already exist
 * @param path the absolute path of the new file
 * @param is_dir true for directory, false for file
 * @return 0 for success, -1 for failure
 */
int makefile(const char *path, bool is_dir);

/**
 * Deletes a file or directory (if empty) given a path
 * @param path the absolute path of the new file
 * @return 0 for success, -1 for failure
 */
int deletefile(const char *path);

#endif
