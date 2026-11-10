[bits 64]

section .text

global set_gdt
set_gdt:
	lgdt [rdi]
	push 0x08
	lea rax, [rel .reload_CS]
    push rax
	retfq
.reload_CS:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret
