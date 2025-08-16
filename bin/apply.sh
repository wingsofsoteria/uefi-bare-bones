#!/usr/bin/env bash
trap "exit 1" SIGINT
if ! test -b /dev/sda1; then
	make
    echo "using fat.img"
    if ! test -f fat.img; then
        echo "creating fat.img"
        dd if=/dev/zero of=fat.img bs=1M count=100
        sync
    fi
    sudo losetup -P loop100 fat.img
    if ! test -f fs.dmp; then
        echo "creating partition table"
        echo -e "label: gpt\n,+,\n" | sudo sfdisk /dev/loop100
        sudo sfdisk -d /dev/loop100 > fs.dmp
    else
    echo "copying partition table from backup"
    sudo sfdisk /dev/loop100 < fs.dmp
    fi
    echo "formatting fat.img"
    sudo mkfs.fat -F32 /dev/loop100p1
    sudo mount /dev/loop100p1 /mnt
    echo "adding files to fat.img"
    sudo mkdir -p /mnt/efi/boot
    sudo cp loader/bootx64.efi /mnt/efi/boot
	sudo cp kernel/kernel /mnt/
	sudo cp initfs /mnt/
	tree /mnt
    sudo umount /mnt
    sudo losetup -d /dev/loop100
else
	make
    echo "using disk"
    sudo mount /dev/sda1 /mnt
	sudo rm -rf /mnt/*
    sudo mkdir -p /mnt/EFI/BOOT
    sudo cp loader/bootx64.efi /mnt/EFI/BOOT
	sudo cp kernel/kernel /mnt/
	sudo cp initfs /mnt/
	tree /mnt
    sudo umount /mnt
fi
