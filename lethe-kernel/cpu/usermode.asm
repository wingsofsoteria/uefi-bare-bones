[bits 64]
section .text
global jump_usermode
jump_usermode:
  cli
  mov rcx, 0xC0000082
  wrmsr
  mov rcx, 0xc0000080
  rdmsr
  or eax, 1
  wrmsr
  mov rcx, 0xC0000081
  mov edx, 0x00100000
  wrmsr
  mov rcx, rdi
  mov r11, 0x202
  o64 sysret
;jump_usermode_old:
;  cli
;  mov ax, 0x23
;  mov ds, ax
;  mov es, ax
;  mov fs, ax
;  mov gs, ax 
;  mov rax, rsp
;  push 0x23
;  push rax
;  sti
;  pushfq
;  push 0x1b
;  push rdi
;  o64 iret
