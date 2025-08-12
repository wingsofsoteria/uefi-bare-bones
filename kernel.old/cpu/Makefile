CC=clang
CFLAGS=-Iinclude -I../libc/include -ffreestanding -fshort-wchar -mno-red-zone  -mcmodel=kernel -fno-stack-protector -mno-mmx -mno-sse -mno-sse2 -fPIC
CPPFLAGS=-mno-mmx -mno-sse -mno-sse2 -fPIC
LDFLAGS= -nostdlib -fpie --static -z max-page-size=0x1000
OBJS=$(patsubst %.c,../objs/%.o,$(wildcard *.c)) $(patsubst %.s,../objs/%.o,$(wildcard *.s))
ifneq ($(QEMU_DEBUG),)
CFLAGS += -DQEMU_DEBUG
endif
all: $(OBJS)

../objs/%.o: %.s
	nasm -felf64 $< -o $@
../objs/%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
