CC=clang
EXEC=bootx64.efi

CFLAGS=-ffreestanding -fshort-wchar -mno-red-zone
LDFLAGS=-ffreestanding -nostdlib
all: $(EXEC)

.PHONY: kernel

$(EXEC): test.elf kernel
	make -C loader

kernel:
	make -C kernel

test.o: test.c
	$(CC) -c $^ -o $@ $(CFLAGS)

test.elf: test.o
	$(CC) $(LDFLAGS) -no-pie $^ -o $@

clean: 
	rm -f fat.img $(EXEC) $(wildcard *.o) elf_tester test.elf
	make -C loader clean
	make -C kernel clean
