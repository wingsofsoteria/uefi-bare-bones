#!/usr/bin/env bash
trap "exit 1" SIGINT
export FEATURE_LIMINE=1
make clean
make
make images
rm test.vdi
VBoxManage convertfromraw --format vdi fat.img "test.vdi" --uuid "472fac57-cc3a-4469-a101-8ef268eaac6b"
