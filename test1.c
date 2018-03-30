#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_malloc.h"

#define STACK_S 4096

int** cancer;

int main(int argc, char *argv[])
{
	/*
	void* sup[24];
	int i = 0;
	for(i = 0; i < 24; i++)
	{
		sup[i] = myallocate(STACK_S, __FILE__, __LINE__, NULL);
	}

	printf("\n\n\n");

	for(i = 23; i >= 0; i--)
	{
		mydeallocate(sup[i], __FILE__, __LINE__, NULL);
	}
	*/
	char * finalTest = (char*)myallocate((sizeof(char) * 12), __FILE__, __LINE__, NULL);
	strcpy(finalTest, "PLEASE");
	printf("%s\n", finalTest);
	mydeallocate(finalTest, __FILE__, __LINE__, NULL);

	return 0;
}
