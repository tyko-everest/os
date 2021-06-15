#include "elf.h"

elf_ec_t parse_elf(const char *path, elf64_header_t *elf_header,
    elf64_prog_header_t *prog_header) {
    
    readfile(path, 0, sizeof(elf64_header_t), elf_header);

    // check if the magic number is correct
    if (!(elf_header->e_ident[EI_MAG0] == 0x7F &&
          elf_header->e_ident[EI_MAG1] == 'E' &&
          elf_header->e_ident[EI_MAG2] == 'L' &&
          elf_header->e_ident[EI_MAG3] == 'F')) {
        
        return ELF_EC_INVL;
    }

    // check if this is a 64 bit elf file
    if (elf_header->e_ident[EI_CLASS] != ELFCLASS64) {
        return ELF_EC_INVL;
    }
    
    readfile(path, elf_header->e_phoff, elf_header->e_phentsize * MIN(elf_header->e_phnum, 2), prog_header);
    return ELF_EC_OK;
}