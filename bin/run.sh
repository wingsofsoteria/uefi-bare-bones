#!/usr/bin/env bash
export QEMU_DEBUG=1
make clean
bin/apply.sh
# ./elf_tester
qemu-system-x86_64 -m 4G -drive if=pflash,format=raw,readonly,file=bin/OVMF_CODE.4m.fd -drive file=fat.img -d int -no-reboot -debugcon stdio  2> >(tee log >&2)
