STRIP=llvm-strip
CC=clang
WARNINGS=-Wno-pointer-to-int-cast -Wno-void-pointer-to-int-cast -Wno-tautological-pointer-compare
CFLAGS=-DKERNEL_USE_LAI -I../lai/include -Iinclude -I../libc/include $(WARNINGS) -O2 -ffreestanding -fshort-wchar -mno-red-zone  -mcmodel=large -fno-stack-protector -mno-mmx -mno-sse -mno-sse2 -fno-PIC -fno-PIE
CPPFLAGS=-mno-mmx -mno-sse -mno-sse2
LDFLAGS=-T linker.lds -nostdlib --static -Wl,-z,max-page-size=0x1000
SRC := $(wildcard *.c */*.c */*/*.c *.s */*.s)
SRC := $(filter-out kernel_end.c, $(SRC))
SRC := $(filter-out kernel_init.c, $(SRC))
OBJS=$(patsubst %,objs/%.o,$(SRC))
ifneq ($(KERNEL_DEBUG),)
CFLAGS += -DKERNEL_DEBUG
endif
all:font kernel
font:
	objcopy -O elf64-x86-64 -B i386 -I binary font.psf objs/font.o 
objs/%.s.o: %.s
	nasm -felf64 -i include $< -o $@
objs/%.c.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
kernel_init.o: kernel_init.c
	$(CC) -c $< -o $@ 
kernel_end.o: kernel_end.c
	$(CC) -c $< -o $@
kernel:kernel_init.o $(OBJS) ../lai/lai.a ../libc/libk.a objs/font.o kernel_end.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@
	$(STRIP) $@
clean: 
	rm -fv $(OBJS) kernel objs/font.o
