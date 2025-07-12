/* x86_64 crtn.s */
.section .init
	popq %rbp
	ret

.section .fini
	popq %rbp
	ret
