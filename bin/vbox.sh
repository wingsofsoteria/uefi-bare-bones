#!/usr/bin/env bash
trap "exit 1" SIGINT
make clean && make && sudo make images
VBoxManage convertfromraw --format vdi fat.img "test.vdi"
