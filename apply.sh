#!/usr/bin/env bash
make clean
make
if ! test -b /dev/sda1; then
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
    sudo mkdir -p /mnt/EFI/BOOT
    sudo cp bootloader/bootx64.efi /mnt/EFI/BOOT
    sudo cp test.elf /mnt/
    sudo umount /mnt
    sudo losetup -d /dev/loop100
else
    echo "using disk"
    sudo mount /dev/sda1 /mnt
    sudo mkdir -p /mnt/EFI/BOOT
    sudo cp bootloader/bootx64.efi /mnt/EFI/BOOT
    sudo cp test.elf /mnt/
    sudo umount /mnt
fi