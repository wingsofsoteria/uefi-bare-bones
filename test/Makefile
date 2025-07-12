CC=clang
CFLAGS=-I. -ffreestanding -g -fshort-wchar -mno-red-zone -fno-stack-protector -mno-mmx -mno-sse -mno-sse2 -fPIC
LDFLAGS=-nostdlib -fpie --static
all: test loader
%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)
test: test.o lib.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@
loader:
	$(CC) -g -o loader loader.c
clean: 
	rm -f $(wildcard *.o) test loader
