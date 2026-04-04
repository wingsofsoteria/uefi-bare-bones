CC=clang
AR=llvm-ar
CFLAGS=-Iinclude -O2 -ffreestanding -fno-stack-protector 
SRC=$(wildcard *.c source/*.c)
OBJS=$(patsubst %,objs/%.o,$(SRC))
all: uacpi.a
objs/%.c.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
uacpi.a: $(OBJS)
	$(AR) rc $@ $^
clean: 
	rm -fv $(OBJS) uacpi.a
