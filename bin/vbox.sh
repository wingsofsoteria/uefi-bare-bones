#!/usr/bin/env bash
trap "exit 1" SIGINT
make clean && make && sudo make images
rm test.vdi
VBoxManage convertfromraw --format vdi fat.img "test.vdi" --uuid "472fac57-cc3a-4469-a101-8ef268eaac6b"
