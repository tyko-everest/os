#ifndef INCLUDE_ATA_H
#define INCLUDE_ATA_H

#include "stdint.h"
#include "port.h"

#include "print.h"

#define ATA_BLOCK_SIZE 512

#define ATA_PRIME 0x1F0
#define ATA_SEC 0x170

#define ATA_DATA_REG(x)         (x)
#define ATA_ERROR_REG(x)        (x + 1)
// used to enable special features
#define ATA_FEATURE_REG(x)      (x + 1)
// specifies the number of sectors to read, 0 is 256
#define ATA_SECTOR_COUNT_REG(x) (x + 2)
#define ATA_SECTOR_NUM_REG(x)   (x + 3)
#define ATA_CYL_LOW_REG(x)      (x + 4)
#define ATA_CYL_HIGH_REG(x)     (x + 5)
#define ATA_HEAD_REG(x)         (x + 6)
#define ATA_STATUS_REG(x)       (x + 7)
#define ATA_COMMAND_REG(x)      (x + 7)
#define ATA_ALT_STATUS_REG(x)   (x + 0x206)
#define ATA_DEV_CONTROL_REG(x)  (x + 0x206)
#define ATA_DRV_ADDR_REG(x)     (x + 0x207)

// these bits always need to be set when writing this register
#define ATA_HEAD_REG_RESERVED_MSK   0b10100000
// setting this bit to 1 is LBA mode, 0 is CHS
#define ATA_HEAD_REG_LBA_MSK        0b01000000
// 0 is master, 1 is slave
#define ATA_HEAD_REG_DRV_MSK        0b00010000
// in LBA this is the highest 4 bits of the address
#define ATA_HEAD_REG_HEAD_MSK       0x00001111

// put error register stuff here

// if BSY or DRQ is set, the drive is busy
#define ATA_STATUS_REG_BSY_MSK      0b10000000
// drive ready
#define ATA_STATUS_REG_DRDY_MSK     0b01000000
// indicates a write fault, not cleared until status register is read
#define ATA_STATUS_REG_DWF_MSK      0b00100000
// disk seek is complete / not seeking
#define ATA_STATUS_REG_DSC_MSK      0b00010000
// set if the drive is ready to send data
// needs to be set to read or write any registers, including command reg
#define ATA_STATUS_REG_DRQ_MSK      0b00001000
// set if a correctable data error was dectected and fixed
#define ATA_STATUS_REG_CORR_MSK     0b00000100
#define ATA_STATUS_REG_IDX_MSK      0b00000010
// set if an error occured, reset when a new command is serviced
#define ATA_STATUS_REG_ERR_MSK      0b00000001

#define ATA_DEV_CONTROL_REG_RESERVED_MSK    0b00001000
#define ATA_DEV_CONTROL_REG_SRST_MSK        0b00000100
#define ATA_DEV_CONTROL_REG_nIEN_MSK        0b00000010

#define ATA_READ_SECTORS    0x20
#define ATA_WRITE_SECTORS   0x30
#define ATA_IDENTIFY        0xEC

typedef enum {
    ATA0 = 0,
    ATA1
} ata_num_t;

typedef enum {
    ATA_MASTER = 0,
    ATA_SLAVE
} ata_type_t;

void ata_pio_read(ata_num_t num, ata_type_t type, uint32_t address,
        uint8_t sector_count, uint16_t* buffer);

void ata_pio_write(ata_num_t num, ata_type_t type, uint32_t address,
        uint8_t sector_count, uint16_t* buffer);

void ata_identify(ata_num_t num, ata_type_t type, uint16_t* buffer);

#endif