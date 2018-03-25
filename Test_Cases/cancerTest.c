#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_pthread_t.h"
#include "my_malloc.h"

pthread_t threads[128];
int indicies[128];

int main(int argc, char *argv[])
{
	int i;

	for(i = 0; i < 128; i++)
	{
		indicies[i] = i+1;
		threads[i] = pthread_create(&threads[i], NULL, threadFunc, &indicies[i]);
	}

	return 0;
}

void *threadFunc(void *arg)
{
	int t = *(int*)arg;

	char *strings[20];

	int i;
	for(i = 0; i < 50; i++)
	{
		strings[i] = (char*)malloc(sizeof(char * 6144));
		sprintf(strings[i], "[Thread %d] String #%d", t, i);
	}

	for(i = 0; i < 50; i++)
	{
		printf("%s\n", strings[i]);
	}

	for(i = 0; i < 50; i+=2)
	{
		free(strings[i]);
	}




	return NULL;
}
