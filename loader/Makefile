CC := clang
LD := lld
ARCH ?= x86-64

CFLAGS := \
	-ffreestanding -MMD -mno-red-zone -std=c11 \
	-target x86_64-unknown-windows -Wall -Werror -pedantic
LDFLAGS := -flavor link -subsystem:efi_application -entry:efi_main

KERNEL_CFLAGS := \
	-ffreestanding -MMD -mno-red-zone -std=c11 \
	-target x86_64-unknown-gnu -Wall -Werror -pedantic
KERNEL_LDFLAGS := \
	-flavor ld -e main


SRCS := main.c clib.c io.c loader.c config.c log.c kernel.c

default: all

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

bootx64.efi: clib.o io.o loader.o config.o log.o main.o
	$(LD) $(LDFLAGS) $^ -out:$@
	mkdir -p ../bootloader
	mv bootx64.efi ../bootloader/

kernel.elf: kernel.c
	$(CC) $(KERNEL_CFLAGS) -c $< -o kernel.o
	$(LD) $(KERNEL_LDFLAGS) kernel.o -o $@

-include $(SRCS:.c=.d)

.PHONY: clean all default

all: bootx64.efi

clean:
	rm -rf ../bootloader
	rm -rf *.efi *.elf *.o *.d *.lib
