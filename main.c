#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void* jit_program;

typedef struct {
	int a;
	int b;
	int result;
	int after;
} jit_global;

static jit_global jit_mem={
	.a=12,
	.b=34,
	.result=1,
	.after=5678,
};


void* jit_func(void* arg) {
	(void) arg;
	int (*func)()=jit_program;
	printf("Here goes nothing...\n");
	long unsigned int res=func();
	printf("Back in normal land! rax=%zu\n",res);
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
	memset(jit_program,0xEB,1); // JMP8
	memset(jit_program+1,0xFE,1); // -2

	unsigned char domath[]={
		0x48, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov addr, %rdi
																		0x57,
		0x48, 0x8B, 0x07, // mov [%rdi], %rax
		0x48, 0x83, 0xC7, 0x04,
		0x48, 0x8B, 0x1F,
		0x48, 0x01, 0xD8,
		0x48, 0x83, 0xC7, 0x04,
		0x89, 0x07,
		0x5F,
		0xEB, 0xFE,
	};

	unsigned char moremath[]={
		0x57,
		0x48, 0x83, 0xC7, 0x08,
		0x48, 0x8B, 0x1F,
		0x48, 0x01, 0xD8,
		0x89, 0x07,
		0x5F,
		0xEB, 0xFE,
	};

	// *(unsigned int*)(domath+2)=(unsigned char*)&jit_mem.a-(unsigned char*)jit_program-6;
	*(unsigned int**)(domath+2)=(unsigned int*)&jit_mem.a;

	printf("a:           %p\n",(unsigned int*)&jit_mem.a);
	printf("jit_program: %p\n",(unsigned int*)jit_program);
	printf("offset:      0x%08x\n",*(unsigned int*)(domath+2));

	printf("&a:      %p\n",&jit_mem.a);
	printf("&b:      %p\n",&jit_mem.b);
	printf("&result: %p\n",&jit_mem.result);
	printf("&after:  %p\n",&jit_mem.after);

	printf("Generated code:\n");
	for(size_t i=0; i<sizeof(domath); i++) {
		printf("%02X ",domath[i]);
	}
	printf("\n");

	pthread_t thread;
	int result=pthread_create(&thread,NULL,jit_func,NULL);
	if(result!=0) {
		fprintf(stderr,"ERROR: cannot create thread\n");
		return 1;
	}

	printf("Sleeping some...\n");
	usleep(100000);

	// writing the code to currently running program
	memcpy(jit_program+2,domath,sizeof(domath));

	printf("Sleeping some cause laziness...\n");
	usleep(100000);
	printf("a=%d, b=%d, result=%d, after=%d\n",jit_mem.a,jit_mem.b,jit_mem.result,jit_mem.after);
	printf("Unleashing the beast\n");
	memset(jit_program,0x9090,2);
	usleep(100000);
	printf("a=%d, b=%d, result=%d, after=%d\n",jit_mem.a,jit_mem.b,jit_mem.result,jit_mem.after);

	printf("Second injection time\n");
	memcpy(jit_program+2+sizeof(domath),moremath,sizeof(moremath));
	printf("Bam!\n");
	printf("a=%d, b=%d, result=%d, after=%d\n",jit_mem.a,jit_mem.b,jit_mem.result,jit_mem.after);
	memset(jit_program+sizeof(domath),0x9090,2);
	usleep(1000); // sleeping some to give assembly time to run
	printf("a=%d, b=%d, result=%d, after=%d\n",jit_mem.a,jit_mem.b,jit_mem.result,jit_mem.after);
	usleep(100000);
	printf("Tellling the VM that it's time to stop\n");
	memset(jit_program+sizeof(domath)+sizeof(moremath),0x9090,2);
	printf("Waiting for thread to finish...\n");
	pthread_join(thread,NULL);
	printf("a=%d, b=%d, result=%d, after=%d\n",jit_mem.a,jit_mem.b,jit_mem.result,jit_mem.after);
	printf("Welp now the program is done running\n");

	munmap(jit_program,size);

	return 0;

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
}
