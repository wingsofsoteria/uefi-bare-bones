CC=clang
AR=llvm-ar
CFLAGS=-Iinclude -O2 -ffreestanding -mcmodel=large -fno-stack-protector
SRC=$(wildcard core/*.c drivers/*.c helpers/*.c)
OBJS=$(patsubst %,objs/%.o,$(SRC))
all: lai.a
objs/%.c.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
lai.a: $(OBJS)
	$(AR) rc $@ $^

clean:
	rm -fv lai.a $(OBJS)
