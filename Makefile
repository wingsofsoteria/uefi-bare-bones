CC=clang
EXEC=bootx64.efi

CFLAGS=-ffreestanding -fshort-wchar -mno-red-zone
LDFLAGS=-ffreestanding -nostdlib
all: $(EXEC) test

.PHONY: kernel test

$(EXEC): kernel
	make -C loader

kernel:
	make -C kernel

test:
	make -C test
clean: 
	rm -f fat.img $(EXEC) $(wildcard *.o)
	make -C loader clean
	make -C kernel clean
	make -C test clean
