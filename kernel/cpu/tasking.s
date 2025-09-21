[bits 64]

%include "cpu/macros.inc"

section .text
global save_task_asm
global load_task_asm
save_task_asm:
  push_x64_regs
  mov rax, rsp
  ret

load_task_asm:
  mov r15, [rdi]
  mov r14, [rdi + 8]
  mov r13, [rdi + 16]
  mov r12, [rdi + 24]
  mov r11, [rdi + 32]
  mov r10, [rdi + 40]
  mov r9, [rdi + 48]
  mov r8, [rdi + 56]
  mov rbp, [rdi + 64]
  mov rsi, [rdi + 80]
  mov rdx, [rdi + 88]
  mov rcx, [rdi + 96]
  mov rbx, [rdi + 104]
  mov rax, [rdi + 112]
  mov rdi, [rdi + 72]
  ret
