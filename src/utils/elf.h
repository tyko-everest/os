#ifndef INCLUDE_ELF_H
#define INCLUDE_ELF_H

#include "clib/stdint.h"
#include "clib/string.h"
#include "system/fat32.h"
#include "utils/macros.h"

// size of e_ident array
#define EI_NIDENT       16
// indexes of bytes in the e_ident array
#define EI_MAG0         0
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3
#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_OSABI        7
#define EI_ABIVERSION   8

// types of classes
#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

// types of data
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

// version of file
#define EV_NONE     0
#define EV_CURRENT  1

// osabi of file
#define ELFOSABI_NONE 0

// types of object files
#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3
#define ET_CORE 4

// machine identifier
#define EM_AARCH64 0xB7

typedef struct {
    uint8_t  e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shtrndx;
} elf64_header_t;

// the program head sections types
#define EPT_LOAD 1

// the program header flag fields
#define EPF_X (1 << 0)
#define EPF_W (1 << 1)
#define EPF_R (1 << 2)

typedef struct {
	uint32_t p_type;
    uint32_t p_flags;
	uint64_t p_offset;
	uintptr_t p_vaddr;
	uintptr_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
} elf64_prog_header_t;

typedef enum {
    ELF_EC_OK   = 0,
    ELF_EC_IO   = -1,
    ELF_EC_INVL = -2
} elf_ec_t;

/**
 * Parses an elf file given its path and will load in the elf header
 * and up to two program headers
 * @param path the path to the file
 * @param elf_header pointer to an elf_header, used to return it
 * @param prog_header pointer to an array of program headers,
 * must be at least two long
 * @return an elf_ec_t, 0 for success, negative for an error
 */
elf_ec_t parse_elf(const char *path, elf64_header_t *elf_header,
    elf64_prog_header_t *prog_header);

#endif //INCLUDE_ELF_H