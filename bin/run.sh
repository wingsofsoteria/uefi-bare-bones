#!/usr/bin/env bash
make clean
bin/apply.sh
# ./elf_tester
qemu-system-x86_64 -m 4G -drive if=pflash,format=raw,readonly,file=bin/OVMF_CODE.4m.fd -drive file=fat.img -debugcon stdio -no-reboot -d int
