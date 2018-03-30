#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_pthread_t.h"


int main(int argc, char *argv[])
{
	char *someString = (char*)malloc(sizeof(char)*12);
	char *someOtherString = (char*)malloc(sizeof(char) * 10);

	strcpy(someString, "Hello");
	strcpy(someOtherString, "Goodbye");

	printf("%s\n", someString);
	printf("%s\n", someOtherString);

	//should this one segfault? idk
	printf("%s\n", someString+18);
	
	free(someString); 
	free(someOtherString);

	return 0;
}

