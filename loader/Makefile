TARGET = bootx64.efi
CFLAGS = -I.. -Wno-all
include uefi/Makefile

clean:
	rm -f $(wildcard *.o) bootx64.efi
	make -C uefi clean
