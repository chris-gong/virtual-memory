#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include "my_pthread_t.h"

int main(int argc, char * argv[])
{
	printf("Size: %d\n", sizeof(tcb));
	printf("Context Size: %d\n", sizeof(ucontext_t));
	fflush(stdout);
}
