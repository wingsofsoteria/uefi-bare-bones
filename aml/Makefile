CC=clang
AR=llvm-ar
CFLAGS=-Iinclude 
ifneq ($(AML_DEBUG),)
CFLAGS += -DAML_DEBUG
else ifneq ($(AML_TEST),)
CFLAGS += -g -DAML_TEST -DAML_DEBUG -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer
endif

ifeq ($(AML_TEST),)
CFLAGS +=-I../libc/include -ffreestanding -fno-stack-protector -mno-red-zone -mcmodel=large -mno-mmx -mno-sse -mno-sse2 -fno-PIC -fno-PIE
endif
SRC=$(wildcard *.c)
OBJS=$(patsubst %,objs/%.o,$(SRC))
all: objs/aml_interpreter.a

objs/%.c.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

objs/aml_interpreter.a: $(OBJS)
	$(AR) rc $@ $^

clean: 
	rm -fv $(OBJS) objs/aml_interpreter.a
