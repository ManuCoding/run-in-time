.section .data
hello:
	.asciz "abcdefg\n"

.section .text
.global funfunc

funfunc:
	push %rdi
	add $8, %rdi
	movq (%rdi), %rbx
	addq %rbx, %rax
	mov %eax, (%rdi)
	pop %rdi
	ret

	add $4,%rdi
	movq (%rdi), %rbx
	addq %rbx, %rax
	addq $4, %rdi
	mov %eax, (%rdi)
	ret
	mov $659706976665, %rdi
	mov (%rdi), %rax
	ret
	mov %rax, %rdi

	mov $60, %rax
	ret
	syscall

	mov $60,%rax
	syscall

