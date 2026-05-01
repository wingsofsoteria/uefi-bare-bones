#!/usr/bin/env bash
set -e
function cleanup() {
tree "$TMP_DIR"
sudo umount "$TMP_DIR"
sudo losetup -d /dev/loop100
}
trap cleanup EXIT
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
if [[ -z "$OUTPUT_IMAGE" ]]; then exit 2; fi
if [[ -z "$KERNEL_FILE" ]]; then exit 1; fi
if [[ -z "$BOOTLOADER_FILES_LIST" ]]; then exit 3; fi
if [[ -z "$BOOTLOADER_DESTINATION" ]]; then exit 5; fi
if [[ -z "$KERNEL_DESTINATION" ]]; then exit 4; fi
if [[ -z "$TMP_DIR" ]]; then exit 6; fi
if [[ ! -e "$KERNEL_FILE" ]]; then exit 7; fi

BOOTLOADER_FILES=$(echo "$BOOTLOADER_FILES_LIST" | tr ";" "\n")
BOOTLOADER_CUSTOM_FILES=$(echo "$BOOTLOADER_CUSTOM_FILES_LIST" | tr ";" "\n")
dd if=/dev/zero of="$OUTPUT_IMAGE" bs=1M count=100
sudo losetup -P loop100 "$OUTPUT_IMAGE"
echo -e "label: gpt\n,+,\n" | sudo sfdisk /dev/loop100
sudo mkfs.fat -F32 /dev/loop100p1
mkdir -p "$TMP_DIR"
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
    exit 1
  else
  for file in $BOOTLOADER_CUSTOM_FILES
  do
    sudo cp "$file" "$BOOTLOADER_CUSTOM_DESTINATION"
  done
  fi
fi

