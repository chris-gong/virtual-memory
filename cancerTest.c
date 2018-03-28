#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_pthread_t.h"

#define THREAD_NUM 85
pthread_t threads[THREAD_NUM];
int indicies[THREAD_NUM];
void *threadFunc(void *);
char* firstMalloc;
int main(int argc, char *argv[])
{
	int i;
        firstMalloc = (char*)malloc(1);
	for(i = 0; i < THREAD_NUM; i++)
	{
		indicies[i] = i+1;
		printf("Creating Thread %d\n", i+1);
		printf("Pthread_Create Status: %d\n", pthread_create(&threads[i], NULL, &threadFunc, &indicies[i]));;
	}

	printf("Main Exiting\n");
	pthread_exit(NULL);

	return 0;
}

void *threadFunc(void *arg)
{
	int t = *(int*)arg;

	char *strings[THREAD_NUM];
	//printf("Running Thread %d\n", t);
        //fflush(stdout);
	int i;
	for(i = 0; i < THREAD_NUM; i++)
	{
		//printf("Thread %d mallocing new string\tIteration %d\n", t, i);
                /*for(j = 0; j < 1000000000; j++)
                {
                  
                }*/
		strings[i] = (char*)malloc(sizeof(char) * 3144);
                //printf("String %s at location %li\n", strings[i], strings[i]-firstMalloc);
		sprintf(strings[i], "[Thread %d] String #%d", t, i);
		//printf("String %s at location %p\n", strings[i], strings[i]);
                //printf("my dick has hiv\n");
	}
	

	pthread_yield();

	int j=0;

	for(j = 0; j < THREAD_NUM; j++)
	{
		printf("Thread %d on loop iteration %d, freeing %s\n",t,j,strings[j]);
		free(strings[j]);
	}

	//printf("Finished freeing in %d\n", t);


	return NULL;
}

