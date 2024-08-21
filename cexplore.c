#include <stdio.h>
#include <stdlib.h>
int number=123;

int funfunc(int* foo);

int main();

int main(void) {
	number=0x1234;
	number+=10;
	int res=funfunc(&number);
	printf("res=%zu\n",res);
	return 0;
}
