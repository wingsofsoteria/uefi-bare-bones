CC=clang
AR=llvm-ar
CFLAGS=-Iinclude -I../kernel/include -ffreestanding -Wall -Wextra -fno-stack-protector -D__is_libk

all: libk.a
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
libk.a: stdio.o string.o stdlib/abort.o
	$(AR) rc $@ $^

clean: 
	rm -f $(wildcard *.o) $(wildcard stdlib/*.o) libk.a
