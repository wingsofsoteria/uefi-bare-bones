#!/usr/bin/env bash
trap "exit 1" SIGINT
export FEATURE_DEBUG=1
export FEATURE_LIMINE=1
make clean
make
make images
make qemu
# ./elf_tester
