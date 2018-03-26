#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_pthread_t.h"

#define THREAD_NUM 16
#define MEM_SIZE (1024 * 1024 * 8)
#define MEM_SECTION (MEM_SIZE/2)

pthread_t threads[THREAD_NUM];
pthread_t extraThreads[THREAD_NUM];
//int indicies[THREAD_NUM];
void *threadFunc(void *);
void *threadFunc2(void*);

int main(int argc, char *argv[])
{
	int pass=1;
        pthread_t id,id2,id3,id4,id5;
	printf("Creating Thread 1\n");
	pthread_create(&id, NULL, threadFunc, &pass);
	int pass2=2;
	printf("Creating Thread 2\n");
	pthread_create(&id2, NULL, threadFunc, &pass2);
	int pass3=3;
	printf("Creating Thread 3\n");
	pthread_create(&id3, NULL, threadFunc, &pass3);
	/*int pass4=4;
	printf("Creating Thread 4\n");
	pthread_create(&id4, NULL, threadFunc, &pass4);
	int pass5=5;
	printf("Creating Thread 5\n");
	pthread_create(&id5, NULL, threadFunc, &pass5);*/
	printf("Main about to finish<<<<<<<<<<<<<<<<<<<ya bitch\n");
	pthread_exit(NULL);

	return 0;
}

void *threadFunc(void *arg)
{

	int t = (*(int*)arg);
	printf("~~~~~~~~~~~~~~~~~~I'm a new Thread %i!~~~~~~~~~~~~~~~~~~~\n", t);
	char* string = (char*)malloc(sizeof(char) * (MEM_SECTION-3));
	sprintf(string, "Bruh, this thread [%d] just allocated!\n",t);
	printf("%s\n",string);
	pthread_yield();

	return NULL;
}
