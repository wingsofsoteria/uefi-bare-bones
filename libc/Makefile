STRIP=llvm-strip
CC=clang
AR=llvm-ar
CFLAGS=-Iinclude -I../kernel/include -O2 -ffreestanding -fno-stack-protector -D__is_libk
ifneq ($(KERNEL_DEBUG),)
CFLAGS += -DKERNEL_DEBUG
endif
all: libk.a
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
%.o: %.s
	nasm -felf64 $< -o $@
libk.a: stdio.o string.o stdlib.o
	$(AR) rc $@ $^
clean: 
	rm -f $(wildcard *.o) $(wildcard stdlib/*.o) libk.a
