CC=clang

ifeq ($(FEATURE_LIMINE),)
all: lethe_loader
images: lethe_image
else
all: limine_loader
images: limine_image
endif

.PHONY: test aml lethe_loader kernel uacpi libc initfs

lethe_loader: objs kernel
	make -C loader
limine_loader: objs kernel
	make -C limine
compiledb: clean
	bear -- make
objs:
	mkdir -p uacpi/objs/source
	mkdir -p kernel/objs/{acpi,cpu,graphics,memory}
uacpi:
	make -C uacpi
libc:
	make -C libc
kernel: uacpi libc
	make -C kernel
clean: 
	rm -f fat.img $(EXEC) $(wildcard *.o) aml_driver/driver
	make -C kernel clean
	make -C libc clean
	make -C uacpi clean
common_image:
	dd if=/dev/zero of=fat.img bs=1M count=100
	sudo losetup -P loop100 fat.img
	echo -e "label: gpt\n,+,\n" | sudo sfdisk /dev/loop100
	sudo mkfs.fat -F32 /dev/loop100p1
	sudo mount /dev/loop100p1 /mnt
	sudo mkdir -p /mnt/efi/boot
lethe_image: common_image
	sudo cp loader/bootx64.efi /mnt/efi/boot
	sudo cp kernel/kernel /mnt/
	sudo cp initfs /mnt/
	tree /mnt
	sudo umount /mnt
	sudo losetup -d /dev/loop100
limine_image: common_image
	sudo mkdir -p /mnt/boot/limine
	sudo cp kernel/kernel /mnt/boot
	sudo cp limine.conf /mnt/boot/limine
	sudo cp limine/limine-bios.sys /mnt/boot/limine
	sudo cp limine/BOOTX64.EFI /mnt/efi/boot
	sudo cp limine/BOOTIA32.EFI /mnt/efi/boot
	tree /mnt
	sudo umount /mnt
	sudo losetup -d /dev/loop100
qemu:
	qemu-system-x86_64 -enable-kvm -cpu host,+invtsc -m 48G -drive if=pflash,format=raw,readonly,file=bin/OVMF.fd -drive file=fat.img -net none -no-reboot -debugcon stdio 1> >(tee log >&2)
