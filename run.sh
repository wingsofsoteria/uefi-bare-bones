#!/usr/bin/env bash
make clean
./apply.sh
# ./elf_tester
qemu-system-x86_64 -drive if=pflash,format=raw,readonly,file=OVMF_CODE.4m.fd -drive file=fat.img -debugcon stdio
