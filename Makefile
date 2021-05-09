CC := arm-none-eabi-gcc
CFLAGS := -c -mcpu=arm1176jzf-s -fpic -ffreestanding -Wall -Wextra -O2
C_INC := -Isrc

AS := arm-none-eabi-as
ASFLAGS := -g

LD := arm-none-eabi-ld
LDFLAGS := -T link.ld -ffreestanding -nostdlib

# setup hdd on ide primary bus master
# setup boot cd on ide primary bus slave
# boot from the cd
QEMU_FLAGS := -curses -m 256 \
	-drive file=disk.img,if=none,format=raw,id=hdd \
	-device ide-hd,drive=hdd,bus=ide.0,unit=0 \
	-drive file=os.iso,if=none,id=bootcd \
	-device ide-cd,drive=bootcd,bus=ide.0,unit=1 \
	-boot d 

SRC_DIR := src
BUILD_DIR := build

C_SRCS := $(wildcard $(SRC_DIR)/**/*.c $(SRC_DIR)/*.c)
C_OBJS := $(subst $(SRC_DIR)/,$(BUILD_DIR)/,$(C_SRCS))
C_OBJS := $(C_OBJS:.c=.o)

AS_SRCS := $(wildcard $(SRC_DIR)/**/*.s $(SRC_DIR)/*.s)
AS_OBJS := $(subst $(SRC_DIR)/,$(BUILD_DIR)/,$(AS_SRCS))
AS_OBJS := $(AS_OBJS:.s=.o)

OBJS = $(C_OBJS) $(AS_OBJS)

dir_guard=@mkdir -p $(@D)

all: $(BUILD_DIR)/kernel7.img

load: $(BUILD_DIR)/kernel7.img
	sudo mount -t drvfs D: /mnt/d
	cp $< /mnt/d/
	sudo umount /mnt/d

$(BUILD_DIR)/kernel7.img: $(BUILD_DIR)/kernel.elf
	arm-none-eabi-objcopy -O binary $< $@

$(BUILD_DIR)/kernel.elf: $(OBJS) $(BUILD_DIR)/zzz.o
	arm-none-eabi-gcc $(LDFLAGS) $^ -o $@ -lgcc

$(BUILD_DIR)/zzz.o: test_fs/zzz.img
	arm-none-eabi-objcopy --rename-section .data=.zzz -I binary -O elf32-littlearm -B arm $< $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) $(C_INC) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	$(dir_guard)
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf build

.PHONY: all clean load
