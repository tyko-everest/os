#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

typedef struct {
    uint32_t pid;
    uint32_t page_desc_addr;
} process_t;

#endif