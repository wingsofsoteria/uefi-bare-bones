[bits 64]
extern kmain
global _start
global _halt
global call_function
section .text

_start:
xor rbp, rbp
call kmain
jmp _halt

_halt:
hlt
jmp _halt

call_function:
	jmp rdi
