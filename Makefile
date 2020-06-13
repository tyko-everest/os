CC := gcc
C_INC :=  -Isrc -Isrc/drivers -Isrc/asm -Isrc/utils -Isrc/system -Isrc/clib
CFLAGS := $(C_INC) \
		 -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
		 -nostartfiles -nodefaultlibs -Wall -Wextra -c -DDEBUG -g
LDFLAGS := -T link.ld -melf_i386
AS := nasm
ASFLAGS := -f elf -F dwarf -g

SRC_DIR := ./src/
BUILD_DIR := ./build/

C_FILES := kmain.c \
	drivers/framebuffer.c \
	drivers/serial.c \
	drivers/keyboard.c \
	drivers/ata.c \
	utils/print.c \
	utils/shell.c \
	utils/elf.c \
	system/gdt.c \
	system/interrupts.c \
	system/kheap.c \
	system/page_frame.c \
	system/file_system.c \
	system/proc.c \
	clib/string.c \
	clib/stdlib.c \
	clib/stdio.c
C_SRCS := $(addprefix $(SRC_DIR)/, $(C_FILES))
C_OBJS := $(addprefix $(BUILD_DIR)/, $(C_FILES))
C_OBJS := $(C_OBJS:.c=.o)

AS_FILES := loader.s \
	asm/port.s \
	asm/gdt.s \
	asm/interrupts.s \
	asm/paging.s
AS_SRCS := $(addprefix $(SRC_DIR)/, $(AS_FILES))
AS_OBJS := $(addprefix $(BUILD_DIR)/, $(AS_FILES))
AS_OBJS := $(AS_OBJS:.s=.o)

OBJS = $(C_OBJS) $(AS_OBJS)

$(info $$OBJS is [${OBJS}])

all: kernel.elf

kernel.elf: $(OBJS)
	ld $(LDFLAGS) $(OBJS) -o kernel.elf

os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R                              \
				-b boot/grub/stage2_eltorito    \
				-no-emul-boot                   \
				-boot-load-size 4               \
				-A os                           \
				-input-charset utf8             \
				-quiet                          \
				-boot-info-table                \
				-o os.iso                       \
				iso

run: os.iso
	bochs -f bochsrc.txt -q

dir_guard=@mkdir -p $(@D)

$(BUILD_DIR)/kmain.o: $(SRC_DIR)/kmain.c
	$(dir_guard)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/loader.o: $(SRC_DIR)/loader.s
	$(dir_guard)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/drivers/%.o: $(SRC_DIR)/drivers/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/utils/%.o: $(SRC_DIR)/utils/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/system/%.o: $(SRC_DIR)/system/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/clib/%.o: $(SRC_DIR)/clib/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/asm/%.o: $(SRC_DIR)/asm/%.s
	$(dir_guard)
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf build kernel.elf os.iso
