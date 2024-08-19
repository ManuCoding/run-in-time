#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void* jit_program;

int a=12;
int b=34;
int result=0;

void* jit_func(void* arg) {
	(void) arg;
	void (*func)()=jit_program;
	printf("Here goes nothing...\n");
	func();
	printf("Back in normal land!\n");
	return NULL;
}

int main() {
	size_t size=4096;
	jit_program=mmap(NULL,size,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	if(jit_program==MAP_FAILED) {
		fprintf(stderr,"ERROR: cannot assign memory\n");
		return 1;
	}
	memset(jit_program,0xC3,size); // fill with RET so that it automatically returns
	// memset(jit_program,0xEB,1); // JMP8
	// memset(jit_program+1,0xFE,1); // -2

	jit_func(NULL);
    unsigned char inject[] = {
        0x48, 0xC7, 0xC0, 0x3C, 0x00, 0x00, 0x00,  // mov rax, 60 (exit syscall)
        0x48, 0xC7, 0xC7, 0x00, 0x00, 0x00, 0x00,  // mov rdi, 0 (exit status)
        0x0F, 0x05                                 // syscall
    };
	memcpy(jit_program,inject,sizeof(inject));
	jit_func(NULL);


    unsigned char code[] = {
        0x8B, 0x05, 0x00, 0x00, 0x00, 0x00,   // mov eax, [rip+a]
        0x03, 0x05, 0x00, 0x00, 0x00, 0x00,   // add eax, [rip+b]
        0x89, 0x05, 0x00, 0x00, 0x00, 0x00    // mov [rip+result], eax
    };

    // Calculate the relative addresses of a, b, and result
    *(unsigned int *)(code + 2) = (unsigned char *)&a - (unsigned char *)jit_program - 6;
    *(unsigned int *)(code + 8) = (unsigned char *)&b - (unsigned char *)jit_program - 12;
    *(unsigned int *)(code + 14) = (unsigned char *)&result - (unsigned char *)jit_program - 18;

	printf("Inserting payload (oooo scary)...\n");
    // Copy the machine code to the executable memory
    memcpy(jit_program, code, sizeof(code));
	jit_func(NULL);
	return 0;

	pthread_t thread;
	int result=pthread_create(&thread,NULL,jit_func,NULL);
	if(result!=0) {
		fprintf(stderr,"ERROR: cannot create thread\n");
		return 1;
	}

	printf("Sleeping some...\n");
	usleep(100000);


	printf("Sleeping some cause laziness...\n");
	usleep(100000);
	printf("Yay result is: %d\n",result);

	printf("Waiting for thread to finish...\n");
	pthread_join(thread,NULL);
	printf("Welp now the program is done running\n");

	munmap(jit_program,size);

	return 0;
}
