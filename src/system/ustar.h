#ifndef USTAR_INCLUDE
#define USTAR_INCLUDE

#include "clib/string.h"
#include "drivers/ata.h"

typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
} ustar_meta_t;

#define USTAR_MAGIC     "ustar"
#define USTAR_MAGIC_LEN 6
#define USTAR_VER       "00"
#define USTAR_VER_LEN   2

int oct2bin(const char *s, int len);

int open(const char *pathname, int flags);

#endif
