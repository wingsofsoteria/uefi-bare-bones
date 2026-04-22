#!/usr/bin/env bash
trap "exit 1" SIGINT
if [ $# -eq 0 ]
then
	echo "Specify a drive to format"
	exit 1
fi
DRIVE=$1
PARTITION="$DRIVE""1"
echo $PARTITION
if [ ! -b $DRIVE ]
then
	echo "$DRIVE is not a block device"
	exit 1
fi
while true; do
	read -p "Are you sure you want to use $DRIVE? [Y/N]" yn
	case $yn in
		[Yy]* ) break;;
		[Nn]* ) exit 1;;
		* ) echo "Invalid input $yn"
	esac
done
echo "using $DRIVE"
export FEATURE_LIMINE=1
export FEATURE_DEBUG=1
make clean
make
make images
if [ ! -f fat.img ]
then
echo "Failed to build Lethe"
exit 1
fi
sudo dd if=fat.img of=$DRIVE status=progress
exit
echo -e "label: gpt\n,+,\n" | sudo sfdisk $DRIVE
sudo mkfs.fat -F32 $PARTITION
sudo mount $PARTITION /mnt
sudo mkdir -p /mnt/efi/boot
sudo mkdir -p /mnt/boot/limine
make clean
make
sudo cp kernel/kernel /mnt/boot
sudo cp limine.conf /mnt/boot/limine
sudo cp limine/limine-bios.sys /mnt/boot/limine
sudo cp limine/BOOTX64.EFI /mnt/efi/boot
sudo cp limine/BOOTIA32.EFI /mnt/efi/boot
tree /mnt
sudo umount /mnt
