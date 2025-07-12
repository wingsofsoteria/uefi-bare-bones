TARGET = bootx64.efi
CFLAGS = -I..
include uefi/Makefile

clean:
	rm -f $(wildcard *.o) bootx64.efi
	make -C uefi clean
