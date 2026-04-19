[bits 64]
extern kmain
global _start
global _halt

section .text

_start:
xor rbp, rbp
call kmain
jmp _halt

_halt:
hlt
jmp _halt
