CC=clang
EXEC=bootx64.efi


all: $(EXEC) test
	clang -g scratch.c
.PHONY: kernel test libc initfs

$(EXEC): kernel initfs
	make -C loader
libc:
	make -C libc
kernel: libc
	make -C kernel
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
