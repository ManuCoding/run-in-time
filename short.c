#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

void* jit_program;

int a=12; // tryna read this value

int main() {
	size_t jit_size=4096;
	jit_program=mmap(NULL,jit_size,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	memset(jit_program,0xC3,jit_size); // fill with RET opcode as initial value

	// assembly that attempts to read the value in global variable a
	unsigned char _read_a[]={
		0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, // mov addr@rip, %eax
	};
	unsigned char __read_a[]={
		0x89, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov [addr], %eax
	};
	unsigned char read_a[]={
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov %rdi, addr
		0x48, 0x8B, 0x07, // mov [%rdi], %rax
		0xC3, // ret
		0x48, 0x89, 0xC7, // mov %rax, %rdi

		0x48, 0xC7, 0xC0, 0x3C, 0x00, 0x00, 0x00,  // mov rax, 60
		0xC3, // ret
		0x0F, 0x05                                 // syscall
	};
	// getting the offset to the value
	unsigned int size_of_instruction=6;
	// *(unsigned int*)(read_a+2)=(unsigned char*)&a-(unsigned char*)jit_program-size_of_instruction;
	int* a_ptr=&a;
	memcpy(read_a+2,&a_ptr,sizeof(a_ptr));
	printf("%p\n",&a);
	for(size_t i=0; i<sizeof(read_a); i++) {
		printf("%02X ",read_a[i]);
	}
	printf("\n");

	// writing the generated program to memory
	memcpy(jit_program,read_a,sizeof(read_a));

	// running the generated code
	int (*func)()=jit_program;
	printf("Running...\n");
	int ret=func();
	printf("Done, ret=%zu\n",ret);
	return 0;
}
