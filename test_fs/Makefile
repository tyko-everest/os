
all:
	gcc -I../src/utils -m32 -g -Wall -DTEST_FS main.c ../src/system/fat32.c

mount:
	sudo losetup --offset 1048576 -f ../zzz.img
	sudo mount /dev/loop0 ../zzz

nukefs:
	sudo umount ../zzz
	sudo mkfs.fat -F 32 /dev/loop0
	sudo mount /dev/loop0 ../zzz

clean:
	rm *.out

