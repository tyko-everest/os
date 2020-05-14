#include "ata.h"

/*
General TODO:
- both read and write lack error checking on transferred data
- read and write 95% the same, make helper function that both call?
*/

void ata_pio_rw_setup() {

}

void ata_pio_read(ata_num_t num, ata_type_t type, uint32_t address,
        uint8_t sector_count, uint16_t* buffer) {

    uint32_t ata_base = 0;
    if (num == ATA0) {
        ata_base = ATA_PRIME;
    } else if (num == ATA1) {
        ata_base = ATA_SEC;
    } else {
        // error
        while(1);
    }
    
    uint8_t head_byte = ATA_HEAD_REG_RESERVED_MSK | ATA_HEAD_REG_LBA_MSK;
    if (type == ATA_MASTER) {
        ;
    } else if (type == ATA_SLAVE) {
        head_byte |= ATA_HEAD_REG_DRV_MSK;
    } else {
        // error
        while(1);
    }
    head_byte |= (address >> 24) & ATA_HEAD_REG_HEAD_MSK;
    

    // wait until the drive is ready, TODO move to ata init function
    while (1) {
        uint8_t status = inb(ATA_ALT_STATUS_REG(ata_base));
        status &= ATA_STATUS_REG_DRDY_MSK;
        if (status) {
            break;
        }
    }

    // disable interrupts
    outb(ATA_DEV_CONTROL_REG(ata_base), ATA_DEV_CONTROL_REG_RESERVED_MSK |
                                        ATA_DEV_CONTROL_REG_nIEN_MSK);

    outb(ATA_HEAD_REG(ata_base), head_byte);

    // 400 ns delay for the device to switch disks if need be
    // TODO only do this when necessary by keeping track of which disk is used
    for (int i = 0; i < 5; i++) {
        inb(ATA_ALT_STATUS_REG(ata_base));
    }

    outb(ATA_FEATURE_REG(ata_base), 0);
    outb(ATA_SECTOR_COUNT_REG(ata_base), sector_count);
    outb(ATA_SECTOR_NUM_REG(ata_base), address & 0xFF);
    outb(ATA_CYL_LOW_REG(ata_base), (address >> 8) & 0xFF);
    outb(ATA_CYL_HIGH_REG(ata_base), (address >> 16) & 0xFF);
    outb(ATA_COMMAND_REG(ata_base), ATA_READ_SECTORS);

    uint32_t buffer_index = 0;

    // transfer the requested number of blocks
    for (uint32_t sector_num = 0; sector_num < sector_count; sector_num++) {

        // poll until the data is ready
        while (1) {
            uint8_t status = inb(ATA_ALT_STATUS_REG(ata_base));
            // uint8_t error = inb(ATA_ERROR_REG(ata_base));
            status &= ATA_STATUS_REG_BSY_MSK | ATA_STATUS_REG_DRQ_MSK;
            if (status == ATA_STATUS_REG_DRQ_MSK) {
                break;
            }
            // TODO add timeout
        }

        for (uint32_t byte_num = 0; byte_num < ATA_BLOCK_SIZE; byte_num++) {
            buffer[buffer_index] = inw(ATA_DATA_REG(ata_base));
            buffer_index++;
        }
        
    }
}

void ata_pio_write(ata_num_t num, ata_type_t type, uint32_t address,
        uint8_t sector_count, uint16_t* buffer) {

    uint32_t ata_base = 0;
    if (num == ATA0) {
        ata_base = ATA_PRIME;
    } else if (num == ATA1) {
        ata_base = ATA_SEC;
    } else {
        // error
        while(1);
    }
    
    uint8_t head_byte = ATA_HEAD_REG_RESERVED_MSK | ATA_HEAD_REG_LBA_MSK;
    if (type == ATA_MASTER) {
        ;
    } else if (type == ATA_SLAVE) {
        head_byte |= ATA_HEAD_REG_DRV_MSK;
    } else {
        // error
        while(1);
    }
    head_byte |= (address >> 24) & ATA_HEAD_REG_HEAD_MSK;
    

    // wait until the drive is ready, TODO move to ata init function
    while (1) {
        uint8_t status = inb(ATA_ALT_STATUS_REG(ata_base));
        status &= ATA_STATUS_REG_DRDY_MSK;
        if (status) {
            break;
        }
    }

    // disable interrupts
    outb(ATA_DEV_CONTROL_REG(ata_base), ATA_DEV_CONTROL_REG_RESERVED_MSK |
                                        ATA_DEV_CONTROL_REG_nIEN_MSK);

    outb(ATA_HEAD_REG(ata_base), head_byte);

    // 400 ns delay for the device to switch disks if need be
    // TODO only do this when necessary by keeping track of which disk is used
    for (int i = 0; i < 5; i++) {
        inb(ATA_ALT_STATUS_REG(ata_base));
    }

    outb(ATA_FEATURE_REG(ata_base), 0);
    outb(ATA_SECTOR_COUNT_REG(ata_base), sector_count);
    outb(ATA_SECTOR_NUM_REG(ata_base), address & 0xFF);
    outb(ATA_CYL_LOW_REG(ata_base), (address >> 8) & 0xFF);
    outb(ATA_CYL_HIGH_REG(ata_base), (address >> 16) & 0xFF);
    outb(ATA_COMMAND_REG(ata_base), ATA_WRITE_SECTORS);

    uint32_t buffer_index = 0;

    // transfer the requested number of blocks
    for (uint32_t sector_num = 0; sector_num < sector_count; sector_num++) {

        // poll until the data is ready
        while (1) {
            uint8_t status = inb(ATA_ALT_STATUS_REG(ata_base));
            // uint8_t error = inb(ATA_ERROR_REG(ata_base));
            status &= ATA_STATUS_REG_BSY_MSK | ATA_STATUS_REG_DRQ_MSK;
            if (status == ATA_STATUS_REG_DRQ_MSK) {
                break;
            }
            // TODO add timeout
        }

        for (uint32_t byte_num = 0; byte_num < ATA_BLOCK_SIZE; byte_num++) {
            outw(ATA_DATA_REG(ata_base), buffer[buffer_index]);
            buffer_index++;
        }
        
    }
    
}