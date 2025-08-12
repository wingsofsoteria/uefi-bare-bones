[bits 64]

global set_idt

section .text

set_idt:
	lidt [rdi]
	ret
