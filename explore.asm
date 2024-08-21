.section .data
hello:
	.asciz "abcdefg\n"

.section .text
.global funfunc

funfunc:
	mov $659706976665, %rdi
	mov (%rdi), %rax
	ret
	mov %rax, %rdi

	mov $60, %rax
	ret
	syscall

	mov $60,%rax
	syscall

