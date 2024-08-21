.section .data
hello:
	.asciz "abcdefg\n"

.section .text
.global funfunc

funfunc:
	mov (%rdi), %rax
	ret
	mov %rax, %rdi

	mov $60, %rax
	ret
	syscall

	mov $60,%rax
	syscall

