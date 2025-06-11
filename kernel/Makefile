CC=clang
CFLAGS=-ffreestanding -fshort-wchar -mno-red-zone -fno-stack-protector -mcmodel=large -mno-mmx -mno-sse -mno-sse2
LDFLAGS=-ffreestanding -nostdlib -no-pie
all: kernel
%.o: %.c
ifeq ($(QEMU_DEBUG),"")
	$(CC) -c $^ -o $@ $(CFLAGS)
else
	$(CC) -c $^ -o $@ $(CFLAGS) -DQEMU_DEBUG
endif

kernel: kernel.o
	$(CC) $(LDFLAGS) $^ -o $@

clean: 
	rm -f *.o kernel
