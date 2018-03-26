#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_pthread_t.h"

#define THREAD_NUM 16

pthread_t threads[THREAD_NUM];
pthread_t extraThreads[THREAD_NUM];
int indicies[THREAD_NUM];
void *threadFunc(void *);
void *threadFunc2(void*);

int main(int argc, char *argv[])
{
/*
	//printf("Beginning of Hell\n");
	indicies[0] = 123;
        indicies[1] = 456;
	pthread_create(&threads[0], NULL, threadFunc, &indicies[0]);
        pthread_create(&threads[1], NULL, threadFunc, &indicies[1]);
	//printf("End of Hell\n");
	pthread_exit(NULL);
*/

	int i;
	for(i = 0; i < THREAD_NUM; i++)
	{
		indicies[i] = i;
		pthread_create(&threads[i], NULL, threadFunc, &indicies[i]);
		//printf("Creating thread %d\n", i+1);
	}

	//printf("Main Exiting...\n");
	pthread_exit(NULL);

	return 0;
}

void *threadFunc(void *arg)
{
	//printf("I'm a new Thread!\n");
	int t = (*(int*)arg)+1;

	char* string = (char*)malloc(sizeof(char) * 4092);
	sprintf(string, "This thread passed: %d", t);
	//printf("%s FROM ADDRESS: %p\n", string, string);
	//free(string);

	////printf("Thread %d creating a child thread\n", t);
	//pthread_create(&extraThreads[t-1], NULL, threadFunc2, &indicies[t-1]);


	//printf("@@@	Thread #%d finished	@@@\n",t);

	return NULL;
}

void *threadFunc2(void *arg)
{
	int parent = *(int*)arg;
	//printf("I am a simple thread. My parent is: %d\n", parent);


	return NULL;
}
