#include "elf.h"

int parse_elf(const char *path, elf64_header_t *elf_header,
    elf64_prog_header_t *prog_header) {
    
    uint8_t buf[1024];
    fat32_readfile(path, 0, 512, buf);

    memcpy(elf_header, buf, sizeof(elf64_header_t));

    // check if the magic number is correct
    if (!(elf_header->e_ident[EI_MAG0] == 0x7F &&
          elf_header->e_ident[EI_MAG1] == 'E' &&
          elf_header->e_ident[EI_MAG2] == 'L' &&
          elf_header->e_ident[EI_MAG3] == 'F')) {
        
        return -1;
    }
    
    memcpy(prog_header, buf + elf_header->e_phoff, elf_header->e_phentsize);
    memcpy(prog_header + 1, buf + elf_header->e_phoff + elf_header->e_phentsize,
        elf_header->e_phentsize);
    
    return 0;
}