#!/usr/bin/env bash
set -e
function cleanup() {
if mountpoint "$TMP_DIR"; then
tree "$TMP_DIR"
sudo umount "$TMP_DIR"
sudo losetup -d /dev/loop100
fi
}
OUTPUT_IMAGE="@LETHE_OUTPUT_FILE@"
KERNEL_FILE="@LETHE_KERNEL_FILE@"
BOOTLOADER_FILES_LIST="@LETHE_BOOTLOADER_FILES@"
BOOTLOADER_CUSTOM_FILES_LIST="@LETHE_BOOTLOADER_CUSTOM_FILES@"
KERNEL_DESTINATION="@LETHE_TEMP_DIR@/@LETHE_KERNEL_DESTINATION@"
BOOTLOADER_DESTINATION="@LETHE_TEMP_DIR@/@LETHE_BOOTLOADER_DESTINATION@"
BOOTLOADER_CUSTOM_DESTINATION="@LETHE_TEMP_DIR@/@LETHE_BOOTLOADER_CUSTOM_DESTINATION@"
TMP_DIR="@LETHE_TEMP_DIR@"
echo "$OUTPUT_IMAGE"
echo "$KERNEL_FILE"
echo "$BOOTLOADER_FILES"
echo "$BOOTLOADER_CUSTOM_FILES"
echo "$BOOTLOADER_DESTINATION"
echo "$KERNEL_DESTINATION"
echo "$BOOTLOADER_CUSTOM_DESTINATION"
echo "$TMP_DIR"
if [[ -b "/dev/loop100" ]]; then 
echo "cleaning up loop device"
cleanup; fi
if [[ -z "$OUTPUT_IMAGE" ]]; then 
echo "no output image specified"
exit 2; 
fi
if [[ -z "$KERNEL_FILE" ]]; then 
echo "no kernel file specified"
exit 1;
fi
if [[ -z "$BOOTLOADER_FILES_LIST" ]]; then 
echo "no bootloader specified"
exit 3;
fi
if [[ -z "$BOOTLOADER_DESTINATION" ]]; then 
echo "no bootloader destination specified"
exit 5; 
fi
if [[ -z "$KERNEL_DESTINATION" ]]; then 
echo "no kernel destination specified"
exit 4; 
fi
if [[ -z "$TMP_DIR" ]]; then 
echo "no tmp dir specified"
exit 6;
fi
if [[ ! -e "$KERNEL_FILE" ]]; then 
echo "kernel file does not exist"
exit 7; 
fi

mkdir -p "$TMP_DIR"
BOOTLOADER_FILES=$(echo "$BOOTLOADER_FILES_LIST" | tr ";" "\n")
BOOTLOADER_CUSTOM_FILES=$(echo "$BOOTLOADER_CUSTOM_FILES_LIST" | tr ";" "\n")
dd if=/dev/zero of="$OUTPUT_IMAGE" bs=1M count=100
sudo losetup -P loop100 "$OUTPUT_IMAGE"
echo -e "label: gpt\n,+,\n" | sudo sfdisk /dev/loop100
sudo mkfs.fat -F32 /dev/loop100p1
sudo mount /dev/loop100p1 "$TMP_DIR"
sudo mkdir -p "$KERNEL_DESTINATION"
sudo mkdir -p "$BOOTLOADER_DESTINATION"
if [[ ! -z "$BOOTLOADER_CUSTOM_DESTINATION" ]];
then
sudo mkdir -p "$BOOTLOADER_CUSTOM_DESTINATION"
fi

sudo cp "$KERNEL_FILE" "$KERNEL_DESTINATION"
for file in $BOOTLOADER_FILES
do
sudo cp "$file" "$BOOTLOADER_DESTINATION"
done
if [[ ! -z "$BOOTLOADER_CUSTOM_FILES_LIST" ]];
then
  if [[ ! -e "$BOOTLOADER_CUSTOM_DESTINATION" ]];
  then
   cleanup 
    exit 1
  else
  for file in $BOOTLOADER_CUSTOM_FILES
  do
    sudo cp "$file" "$BOOTLOADER_CUSTOM_DESTINATION"
  done
  fi
fi
cleanup
