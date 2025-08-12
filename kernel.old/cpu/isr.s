[bits 64]



section .text

extern exception_handler
extern interrupt_handler

%macro isr_exception_stub 1
isr_stub_%+%1:
	cli
    call exception_handler
    iretq
%endmacro


%macro isr_no_ret_stub 1
isr_stub_%+%1:
	cli
    call exception_handler
	jmp no_return_loop
%endmacro

%macro isr_int_stub 1
isr_stub_%+%1:
	call interrupt_handler
	iretq
%endmacro



global no_return_loop
no_return_loop:
	hlt
	jmp no_return_loop

isr_no_ret_stub 0
isr_no_ret_stub 1
isr_no_ret_stub 2
isr_no_ret_stub 3
isr_no_ret_stub 4
isr_no_ret_stub 5
isr_no_ret_stub 6
isr_no_ret_stub 7
isr_no_ret_stub 8
isr_no_ret_stub 9
isr_no_ret_stub 10
isr_no_ret_stub 11
isr_no_ret_stub 12
isr_no_ret_stub 13
isr_no_ret_stub 14
isr_no_ret_stub 15
isr_no_ret_stub 16
isr_no_ret_stub 17
isr_no_ret_stub 18
isr_no_ret_stub 19
isr_no_ret_stub 20
isr_no_ret_stub 21
isr_no_ret_stub 22
isr_no_ret_stub 23
isr_no_ret_stub 24
isr_no_ret_stub 25
isr_no_ret_stub 26
isr_no_ret_stub 27
isr_no_ret_stub 28
isr_no_ret_stub 29
isr_no_ret_stub 30
isr_no_ret_stub 31

;%assign i 32
;%rep 224
;isr_int_stub i
;%assign i i+1
;%endrep

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32
    dq isr_stub_%+i
%assign i i+1 
%endrep

global set_idt
set_idt:
    lidt [rdi]
	sti
    ret
