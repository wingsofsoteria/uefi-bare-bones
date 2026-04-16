CC=clang
AR=llvm-ar
CFLAGS=-Iinclude -I../kernel/include -ffreestanding -fno-stack-protector -D__is_libk -mno-mmx -mno-sse -mno-sse2
ifneq ($(FEATURE_DEBUG),)
CFLAGS += -DKERNEL_DEBUG -g
else
CFLAGS += -O2
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
