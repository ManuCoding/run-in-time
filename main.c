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
	unsigned char domath[]={
		0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, // mov addrthingy, eax
	};

	*(unsigned int*)(domath+2)=(unsigned char*)&a-(unsigned char*)jit_program-6;

	printf("a:           %p\n",(unsigned int*)&a);
	printf("jit_program: %p\n",(unsigned int*)jit_program);
	printf("offset:      0x%08x\n",*(unsigned int*)(domath+2));


	printf("Generated code:\n");
	for(size_t i=0; i<sizeof(domath); i++) {
		printf("%02X ",domath[i]);
	}
	printf("\n");

	memcpy(jit_program,domath,sizeof(domath));

	jit_func(NULL);

	// debugging code above so the rest here is ignored, code below works
	unsigned char inject[] = {
		0x48, 0xC7, 0xC0, 0x3C, 0x00, 0x00, 0x00,  // mov rax, 60
		0x48, 0xC7, 0xC7, 0x2A, 0x00, 0x00, 0x00,  // mov rdi, 42
		0x0F, 0x05                                 // syscall
	};
	// successfully makes the program return with 42
	memcpy(jit_program,inject,sizeof(inject));
	jit_func(NULL);

	return 0;

	// thread code, currently removed cause testing stuff
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
