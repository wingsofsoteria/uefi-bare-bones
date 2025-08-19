CC=clang
WARNINGS=-Wno-pointer-to-int-cast -Wno-void-pointer-to-int-cast -Wno-tautological-pointer-compare
CFLAGS=-Iinclude -I../libc/include $(WARNINGS) -ffreestanding -fshort-wchar -mno-red-zone  -mcmodel=large -fno-stack-protector -mno-mmx -mno-sse -mno-sse2 -fno-PIC -fno-PIE
CPPFLAGS=-mno-mmx -mno-sse -mno-sse2
LDFLAGS=-T linker.lds -nostdlib --static -Wl,-z,max-page-size=0x1000
SRC=$(wildcard *.c */*.c *.s */*.s)
OBJS=$(patsubst %,objs/%.o,$(SRC))
ifneq ($(QEMU_DEBUG),)
CFLAGS += -DQEMU_DEBUG
endif
all:font kernel
font:
	objcopy -O elf64-x86-64 -B i386 -I binary font.psf objs/font.o 
objs/%.s.o: %.s
	nasm -felf64 $< -o $@
objs/%.c.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
kernel:$(OBJS) ../libc/libk.a objs/font.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@

clean: 
	rm -f $(OBJS) kernel objs/font.o
