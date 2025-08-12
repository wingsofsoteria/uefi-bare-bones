[bits 64]

%macro push_x64_regs 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro pop_x64_regs 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro isr_hack 1
	dq isr_stub_%1
%endmacro
%macro isr_err_stub 1
isr_stub_%1:
	push qword %1
	jmp interrupt_common
%endmacro

%macro isr_no_err_stub 1
isr_stub_%1:
	push qword 0
	push qword %1
	jmp interrupt_common
%endmacro

extern exception_handler

section .text
interrupt_common:
	push_x64_regs
	mov rbp, rsp
	mov rdi, rsp
	call exception_handler
	pop_x64_regs
	add rsp, 16
	iretq

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

%assign n 32
%rep 224
isr_no_err_stub n
%assign n  n+1
%endrep

section .data

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
	isr_hack i
%assign i i+1
%endrep
