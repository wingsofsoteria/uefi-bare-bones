#!/usr/bin/env bash
trap "exit 1" SIGINT
export KERNEL_DEBUG=1
bin/apply.sh
# ./elf_tester
qemu-system-x86_64 -accel kvm -cpu host,+invtsc -m 4G -drive if=pflash,format=raw,readonly=on,file=bin/OVMF.fd -drive format=raw,file=fat.img -net none -no-reboot -d int -debugcon stdio 2> >(tee log >&2)
