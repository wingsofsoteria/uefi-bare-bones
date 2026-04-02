CC=clang
EXEC=bootx64.efi


all: $(EXEC)
.PHONY: test aml kernel libc initfs

$(EXEC): kernel
	make -C loader
aml:
	make -C aml
libc:
	make -C libc
kernel: aml libc
	make -C kernel

test:
	AML_TEST=1 make -C aml
	$(CC) aml_driver/driver.c aml/objs/aml_interpreter.a -g -o aml_driver/driver

clean: 
	rm -f fat.img $(EXEC) $(wildcard *.o) aml_driver/driver
	make -C loader clean
	make -C kernel clean
	make -C libc clean
	make -C loader/uefi clean
	make -C aml clean
images:
	dd if=/dev/zero of=fat.img bs=1M count=100
	losetup -P loop100 fat.img
	echo -e "label: gpt\n,+,\n" | sfdisk /dev/loop100
	mkfs.fat -F32 /dev/loop100p1
	mount /dev/loop100p1 /mnt
	mkdir -p /mnt/efi/boot
	cp loader/bootx64.efi /mnt/efi/boot
	cp kernel/kernel /mnt/
	cp initfs /mnt/
	tree /mnt
	umount /mnt
	losetup -d /dev/loop100
qemu: images
	qemu-system-x86_64 -m 4G -drive if=pflash,format=raw,readonly,file=bin/OVMF_CODE.4m.fd -drive file=fat.img -d int -no-reboot -monitor stdio 2> >(tee log >&2)
