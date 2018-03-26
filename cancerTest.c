#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_pthread_t.h"

pthread_t threads[128];
int indicies[128];
void *threadFunc(void *);
char* firstMalloc;
int main(int argc, char *argv[])
{
	int i;
        firstMalloc = (char*)malloc(1)-(sizeof(char)*3);
        free(firstMalloc);
	for(i = 0; i < 128; i++)
	{
		indicies[i] = i+1;
		printf("Creating Thread %d\n", i+1);
		threads[i] = pthread_create(&threads[i], NULL, &threadFunc, &indicies[i]);
	}

	pthread_exit(NULL);

	return 0;
}

void *threadFunc(void *arg)
{
	int t = *(int*)arg;

	char *strings[50];
	printf("Running Thread %d\n", t);
        fflush(stdout);
	int i;
	for(i = 0; i < 50; i++)
	{
		printf("Thread %d mallocing new string\tIteration %d\n", t, i);
                int j;
                /*for(j = 0; j < 1000000000; j++)
                {
                  
                }*/
		strings[i] = (char*)malloc(sizeof(char) * 3144);
                printf("String %s at location %i\n", strings[i], strings[i]-firstMalloc);
		sprintf(strings[i], "[Thread %d] String #%d", t, i);
		//printf("String %s at location %p\n", strings[i], strings[i]);
                //printf("my dick has hiv\n");
	}

	pthread_yield();

	//printf("Finished mallocing\n");

	for(i = 0; i < 50; i++)
	{
		printf("%s\n", strings[i]);
	}

	//printf("Finished printing\n");

	for(i = 0; i < 50; i+=2)
	{
		free(strings[i]);
	}

	//printf("Finished freeing\n");


	return NULL;
}
