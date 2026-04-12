STRIP=llvm-strip
CC=clang
OBJCOPY=objcopy
RM=rm
CAT=cat
DECOMPRESS=gunzip -c > font.psf
WARNINGS=-Wno-pointer-to-int-cast -Wno-void-pointer-to-int-cast -Wno-tautological-pointer-compare
INCLUDE=-Iinclude -I../lai/include -I../libc/include
CFLAGS=$(INCLUDE) $(WARNINGS) -O2 -ffreestanding -fshort-wchar -mno-red-zone  -mcmodel=large -fno-stack-protector -mno-mmx -mno-sse -mno-sse2 -fno-PIC -fno-PIE
CPPFLAGS=-mno-mmx -mno-sse -mno-sse2
LDFLAGS=-nostdlib --static -Wl,-z,max-page-size=0x1000
#SRC := $(wildcard *.c */*.c */*/*.c *.s */*.s)
#SRC := $(filter-out kernel_end.c, $(SRC))
#SRC := $(filter-out kernel_init.c, $(SRC))
#OBJS=$(patsubst %,objs/%.o,$(SRC))
ifneq ($(FEATURE_DEBUG),)
KERNEL_CFLAGS += -DKERNEL_DEBUG
endif
ifneq ($(FEATURE_LIMINE),)
CFLAGS += -DKERNEL_USE_LIMINE
LDFLAGS += -T scripts/limine_loader.lds 
else
LDFLAGS += -T scripts/lethe_loader.lds
endif
FONT=/usr/share/consolefonts/default8x16.psfu.gz
all:kernel
%.c.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
%.s.o: %.s
	nasm -felf64 -i include $< -o $@
OBJS=kernel_init.o font.o
CLEAN_OBJS = kernel_init.o kernel.o kernel_end.o
include makefiles/*.mk
OBJS += kernel.o ../lai/lai.a ../libc/libk.a kernel_end.o
font.o:
	$(CAT) $(FONT) | $(DECOMPRESS)
	$(OBJCOPY) -O elf64-x86-64 -B i386 -I binary font.psf font.o 
kernel_init.o: kernel_init.c
	$(CC) -c $< -o $@
kernel_end.o: kernel_end.c
	$(CC) -c $< -o $@
kernel:$(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@
	$(STRIP) $@
clean: 
	$(RM) -fv $(CLEAN_OBJS)
