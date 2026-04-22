CC=clang
ifeq ($(FEATURE_LIMINE),)
all: lethe_loader
images: lethe_image
else
all: limine_loader
images: limine_image
endif

IMAGE=kernel/kernel
ifneq ($(FEATURE_DEBUG),)
IMAGE=kernel/debug_kernel
endif
QEMU_CPU=-enable-kvm -cpu host,+invtsc
QEMU_MEM=-m 48G
QEMU_FW=-drive if=pflash,format=raw,readonly=on,unit=0,file=bin/OVMF_CODE.fd -drive if=pflash,format=raw,readonly=on,unit=1,file=bin/OVMF_VARS.fd 
QEMU_HDD=-drive file=fat.img
ifneq ($(FEATURE_QEMU),)
QEMU_LOG=-debugcon stdio 1> >(tee log >&2)
else
QEMU_LOG=-monitor stdio 1> >(tee log >&2)
endif
QEMU_FLAGS=-net none -no-reboot $(QEMU_CPU) $(QEMU_MEM) $(QEMU_FW) $(QEMU_HDD) $(QEMU_LOG)

.PHONY: objs test aml lethe_loader lethe_image limine_image limine_loader common_image kernel lai libc initfs
lethe_loader: objs kernel
	make -C loader
limine_loader: objs kernel
	make -C limine

compiledb: clean
	bear -- make
objs:
	mkdir -p lai/objs/{core,drivers,helpers}
	mkdir -p kernel/objs/{acpi,cpu,graphics,memory}

lai:
	make -C lai
libc:
	make -C libc
kernel: lai libc
	make -C kernel
common_image:
	dd if=/dev/zero of=fat.img bs=1M count=100
	sudo losetup -P loop100 fat.img
	echo -e "label: gpt\n,+,\n" | sudo sfdisk /dev/loop100
	sudo mkfs.fat -F32 /dev/loop100p1
	sudo mount /dev/loop100p1 /mnt
	sudo mkdir -p /mnt/efi/boot
lethe_image: common_image
	sudo cp loader/bootx64.efi /mnt/efi/boot
	sudo cp $(IMAGE) /mnt/kernel
	sudo cp initfs /mnt/
	tree /mnt
	sudo umount /mnt
	sudo losetup -d /dev/loop100
limine_image: common_image
	sudo mkdir -p /mnt/boot/limine
	sudo cp $(IMAGE) /mnt/boot/kernel
	sudo cp limine.conf /mnt/boot/limine
	sudo cp limine/limine-bios.sys /mnt/boot/limine
	sudo cp limine/BOOTX64.EFI /mnt/efi/boot
	sudo cp limine/BOOTIA32.EFI /mnt/efi/boot
	tree /mnt
	sudo umount /mnt
	sudo losetup -d /dev/loop100
qemu:
	qemu-system-x86_64 $(QEMU_FLAGS)	

clean: 
	rm -f fat.img $(EXEC) $(wildcard *.o) aml_driver/driver
	make -C kernel clean
	make -C libc clean
	make -C lai clean
