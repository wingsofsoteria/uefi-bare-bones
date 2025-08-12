CC=clang
AR=llvm-ar
CFLAGS=-Iinclude -I../kernel/include -ffreestanding -mgeneral-regs-only  -Wall -Wextra -fno-stack-protector -D__is_libk

all: libk.a
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
%.o: %.s
	nasm -felf64 $< -o $@
libk.a: stdio.o string.o stdlib.o
	$(AR) rc $@ $^

clean: 
	rm -f $(wildcard *.o) $(wildcard stdlib/*.o) libk.a
