#ifndef INCLUDE_ELF_H
#define INCLUDE_ELF_H

#include "clib/stdint.h"
#include "clib/string.h"
#include "system/fat32.h"

#define EI_NIDENT 16
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8

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

// currently only reads the first two program headers
int parse_elf(const char *path, elf64_header_t *elf_header,
    elf64_prog_header_t *prog_header);

#endif //INCLUDE_ELF_H