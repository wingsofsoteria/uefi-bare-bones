CC=clang
EXEC=bootx64.efi

CFLAGS=-ffreestanding -fshort-wchar -mno-red-zone
LDFLAGS=-ffreestanding -nostdlib
all: $(EXEC) test

.PHONY: kernel test libc

$(EXEC): kernel initfs
	USE_GCC=1 make -C loader
libc:
	make -C libc
kernel: libc
	make -C kernel
initfs: font
	echo font | cpio -o > initfs
font:
	clang gen_font.c
	./a.out
	rm ./a.out
test:
	make -C test
clean: 
	rm -f fat.img $(EXEC) $(wildcard *.o) initfs
	make -C loader clean
	make -C kernel clean
	make -C test clean
	make -C libc clean
