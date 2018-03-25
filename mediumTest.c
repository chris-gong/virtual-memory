#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_pthread_t.h"

pthread_t threads[16];
int indicies[16];
void *threadFunc(void*);

int main(int argc, char *argv[])
{
	int i;

	for(i = 0; i < 1; i++)
	{
		indicies[i] = i+1;
		threads[i] = pthread_create(&threads[i], NULL, threadFunc, &indicies[i]);
	}

	return 0;
}

void *threadFunc(void *arg)
{
	 printf("Test...\n");
	int t = *(int*)arg;

	char *strings[20];
	printf("Farther?\n");
	int i;
	for(i = 0; i < 50; i++)
	{
		printf("In the loop\n");
		strings[i] = (char*)malloc(sizeof(char)*32);
		sprintf(strings[i], "[Thread %d] String #%d", t, i);
	}

	for(i = 0; i < 50; i++)
	{
		printf("-->%s\n", strings[i]);
	}

	for(i = 0; i < 50; i+=2)
	{
		free(strings[i]);
	}




	return NULL;
}
