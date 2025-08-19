CC=clang
EXEC=bootx64.efi


all: $(EXEC)
	clang -g scratch.c
.PHONY: kernel libc initfs

$(EXEC): kernel
	make -C loader
libc:
	make -C libc
kernel: libc
	make -C kernel
clean: 
	rm -f fat.img $(EXEC) $(wildcard *.o)
	make -C loader clean
	make -C kernel clean
	make -C libc clean
