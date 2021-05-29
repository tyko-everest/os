PREFIX := aarch64-none-elf

CC := ${PREFIX}-gcc
CFLAGS := -mstrict-align -nostdlib -ffreestanding -Wall -Wextra -O2 -T link.ld -z max-page-size=4096
C_INC := -Isrc

PRG: prg.c
	$(CC) $(CFLAGS) $(C_INC) $< -o $@

clean:
	rm PRG

print: PRG
	aarch64-none-elf-objdump -p PRG


.PHONY: clean print