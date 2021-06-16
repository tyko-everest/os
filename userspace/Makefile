PREFIX := aarch64-none-elf

CC := ${PREFIX}-gcc
CFLAGS := -mstrict-align -nostdlib -ffreestanding -fno-pic -Wall -Wextra -Og -T link.ld -z max-page-size=4096
C_INC := -Isrc

PRG: prg.c start.s
	$(CC) $(CFLAGS) $(C_INC) $^ -o $@

PRG2: prg2.c start.s
	$(CC) $(CFLAGS) $(C_INC) $^ -o $@

add_to_fs: PRG PRG2
	make -C ../test_fs mount
	sudo cp $^ ../test_fs/zzz
	make -C ../test_fs umount

clean:
	rm PRG*

print: PRG
	aarch64-none-elf-objdump -p PRG


.PHONY: clean print