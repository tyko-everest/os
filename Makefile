PREFIX := aarch64-none-elf

CC := ${PREFIX}-gcc
CFLAGS := -c -mstrict-align -fpic -ffreestanding -Wall -Wextra -Og -g
C_INC := -Isrc

RSC := rustc
RSFLAGS := --target aarch64-unknown-none --crate-type staticlib --emit obj -g

ZIGC := zig build-obj
ZIGFLAGS := -target aarch64-freestanding-none -isystem src

AS := ${PREFIX}-as
ASFLAGS := -g

LD := ${PREFIX}-ld
LDFLAGS := -T link.ld -ffreestanding -nostdlib

SRC_DIR := src
BUILD_DIR := build

C_SRCS := $(wildcard $(SRC_DIR)/**/*.c $(SRC_DIR)/*.c)
C_OBJS := $(subst $(SRC_DIR)/,$(BUILD_DIR)/,$(C_SRCS))
C_OBJS := $(C_OBJS:.c=.o)

RS_SRCS := $(wildcard $(SRC_DIR)/**/*.rs $(SRC_DIR)/*.rs)
RS_OBJS := $(subst $(SRC_DIR)/,$(BUILD_DIR)/,$(RS_SRCS))
RS_OBJS := $(RS_OBJS:.rs=.o)

ZIG_SRCS := $(wildcard $(SRC_DIR)/**/*.zig $(SRC_DIR)/*.zig)
ZIG_OBJS := $(subst $(SRC_DIR)/,$(BUILD_DIR)/,$(ZIG_SRCS))
ZIG_OBJS := $(ZIG_OBJS:.zig=.o)

AS_SRCS := $(wildcard $(SRC_DIR)/**/*.s $(SRC_DIR)/*.s)
AS_OBJS := $(subst $(SRC_DIR)/,$(BUILD_DIR)/,$(AS_SRCS))
AS_OBJS := $(AS_OBJS:.s=.o)

OBJS = $(C_OBJS) $(RS_OBJS) $(AS_OBJS)

QEMU_FLAGS := -M raspi3b -vnc :0 \
	-kernel $(BUILD_DIR)/kernel8.img \
	-serial null -serial stdio

dir_guard=@mkdir -p $(@D)

all: $(BUILD_DIR)/kernel8.img

load: $(BUILD_DIR)/kernel8.img
	sudo mount -t drvfs E: /mnt/e
	cp $< /mnt/e/
	sudo umount /mnt/e

run: $(BUILD_DIR)/kernel8.img
	qemu-system-aarch64 $(QEMU_FLAGS)

debug: $(BUILD_DIR)/kernel8.img
	qemu-system-aarch64 -s -S $(QEMU_FLAGS)

$(BUILD_DIR)/kernel8.img: $(BUILD_DIR)/kernel.elf
	${PREFIX}-objcopy -O binary $< $@

$(BUILD_DIR)/kernel.elf: $(OBJS) $(BUILD_DIR)/zzz.o
	${CC} $(LDFLAGS) $^ -o $@ -lgcc

$(BUILD_DIR)/zzz.o: test_fs/zzz.img
	${PREFIX}-objcopy --rename-section .data=.zzz -I binary -O elf64-littleaarch64 $< $@

# $(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.c
# 	$(dir_guard)
# 	$(CC) $(CFLAGS) $(C_INC) $< -o $@

# $(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.rs
# 	$(dir_guard)
# 	$(RSC) $(RSFLAGS) $< -o $@

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.zig
	$(dir_guard)
	$(ZIGC) $(ZIGFLAGS) $< -femit-bin=$@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(dir_guard)
	$(CC) $(CFLAGS) $(C_INC) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	$(dir_guard)
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -r build

.PHONY: all clean load run debug
