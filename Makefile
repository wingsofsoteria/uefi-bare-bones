CC=clang
DD=dd
FORMAT=mformat
MKDIR=mmd
COPY=mcopy
QEMU=qemu-system-x86_64

TARGET=x86_64-unknown-windows
EXEC=bootx64.efi
OBJS=loader.o
IMG=fat.img
FIRMWARE=$(FDIR)/OVMF_CODE.fd

FDIR=/usr/share/OVMF
IDIRS=/usr/include /usr/include/efi/x86_64 /usr/include/efi/protocol

INCLUDE=$(foreach dir,$(IDIRS),-I$(dir))
CFLAGS=-target $(TARGET) $(INCLUDE) -ffreestanding -fshort-wchar -mno-red-zone
LDFLAGS=-ffreestanding -nostdlib
QFLAGS=-L $(FDIR) -pflash $(FIRMWARE) -drive file=$(IMG),format=raw,index=0,media=disk
all: $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(EXEC): test.elf
	make -C loader

run: $(IMG)
	$(QEMU) $(QFLAGS)

test.o: test.c
	$(CC) -c $^ -o $@ -ffreestanding -fshort-wchar -mno-red-zone
test.elf: test.o
	$(CC) $(LDFLAGS) -no-pie $^ -o $@

$(IMG): $(EXEC) 
	dd if=/dev/zero of=$(IMG) bs=1k count=1440
	mformat -i $(IMG) -f 1440 ::
	mmd -i $(IMG) ::/efi
	mmd -i $(IMG) ::/efi/boot
	mcopy -i $(IMG) $(EXEC) ::/efi/boot
	mcopy -i $(IMG) test.elf ::/

clean: 
	rm -f $(IMG) $(EXEC) $(wildcard *.o) elf_tester test.elf
	make -C loader clean
	make -C loader/uefi clean