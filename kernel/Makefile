CC=clang
CFLAGS=-Iinclude -I.. -ffreestanding -fshort-wchar -mno-red-zone -fno-stack-protector -mno-mmx -mno-sse -mno-sse2 -fPIC -g
CPPFLAGS=-mno-mmx -mno-sse -mno-sse2 -fPIC
LDFLAGS=-nostdlib -fpie --static
ifneq ($(QEMU_DEBUG),)
CFLAGS += -DQEMU_DEBUG
endif
all: kernel
%.o: %.s
	$(CC) -c $< -o $@ $(CPPFLAGS)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
kernel: crti.o kernel.o ../libc/libk.a qemu.o idt.o gdt.o cpio.o pages.o graphics/pixel.o graphics/tty.o crtn.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@

clean: 
	rm -f $(wildcard *.o) $(wildcard graphics/*.o) kernel
