.section .data
hello:
	.asciz "abcdefg\n"

.section .text
.global _start

_start:
	mov $1,%rax
	mov $1,%rdi
	mov $hello,%rsi
	mov $8,%rdx
	syscall

	mov $60,%rax
	mov $305419896,%rbx
	mov $42,%rdi
	syscall

