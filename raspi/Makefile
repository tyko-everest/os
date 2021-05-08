CC := arm-none-eabi-gcc
CFLAGS := -c -mcpu=arm1176jzf-s -fpic -ffreestanding -Wall -Wextra -O2

AS := arm-none-eabi-as
ASFLAGS := -g

LD := arm-none-eabi-ld
LDFLAGS := -T kernel.ld -ffreestanding -nostdlib

load: kernel7.img
	sudo mount -t drvfs D: /mnt/d
	cp kernel7.img /mnt/d/
	sudo umount /mnt/d

all: kernel7.img

kernel7.img: kernel.elf
	arm-none-eabi-objcopy -O binary kernel.elf kernel7.img

kernel.elf: start.o kernel.o printf.o
	arm-none-eabi-gcc $(LDFLAGS) $^ -o $@ -lgcc

kernel.o: kernel.c
	$(CC) $(CFLAGS) $< -o $@

printf.o: printf.c
	$(CC) $(CFLAGS) $< -o $@

start.o: start.s
	$(AS) $(ASFLAGS) $< -o $@

sd.o: sd.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm *.o *.img *.elf

