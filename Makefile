CC=clang
EXEC=bootx64.efi

CFLAGS=-ffreestanding -fshort-wchar -mno-red-zone
LDFLAGS=-ffreestanding -nostdlib
all: $(EXEC) test

.PHONY: kernel test libc initfs

$(EXEC): kernel initfs
	make -C loader
libc:
	make -C libc
kernel: libc
	QEMU_DEBUG=1 make -C kernel
initfs: font
	cpio -o < initfs_list > initfs
font:
	clang gen_font.c
	./a.out
	rm ./a.out
test:
	make -C test
clean: 
	rm -f fat.img $(EXEC) $(wildcard *.o)
	make -C loader clean
	make -C kernel clean
	make -C test clean
	make -C libc clean
