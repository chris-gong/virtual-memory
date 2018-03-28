#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_pthread_t.h"

#define THREAD_NUM 16
#define MEM_SIZE (1024 * 1024 * 8)
#define MEM_SECTION (MEM_SIZE/2)
#define PAGE_SIZE (1024 * 4)

pthread_t threads[THREAD_NUM];
pthread_t extraThreads[THREAD_NUM];
//int indicies[THREAD_NUM];
char *s1;
char *s2;
char *s3;
char *s4;
char *s5;
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
	int pass4=4;
	printf("Creating Thread 4\n");
	pthread_create(&id4, NULL, threadFunc, &pass4);
	int pass5=5;
	printf("Creating Thread 5\n");
	pthread_create(&id5, NULL, threadFunc, &pass5);
	pthread_join(id,NULL);
	pthread_join(id2,NULL);
	pthread_join(id3,NULL);
	pthread_join(id4,NULL);
	pthread_join(id5,NULL);
	printf("Main has come to free everyone!\n");
	free(s1);
	free(s2);
	free(s3);
	free(s4);
	free(s5);
	printf("It is done.\n");
	pthread_exit(NULL);

	return 0;
}

void *threadFunc(void *arg)
{

	int t = (*(int*)arg);
	printf("~~~~~~~~~~~~~~~~~~I'm a new Thread %i!~~~~~~~~~~~~~~~~~~~\n", t);
	char* string = (char*)malloc(sizeof(char) * (5));
	//sprintf(string, "This string belongs to: Thread #%d\n",t);
	char* string2;
        if (t == 5)
	{
		string2 = (char*)shalloc(sizeof(char)*16345);
		//sprintf(string2, "Woah %d, calm down man\n",t);
		printf("%s\n",string2);
		s5=string2;
		/*printf("%d is about to free everyone!\n",t);
		free(s1);
		free(s2);
		free(s3);
		free(s4);
		free(string2);*/
	}
	else
	{
        	string2 = (char*)shalloc(sizeof(char)*6);
	        sprintf(string2, "Yo %d\n",t);
		printf("%s\n",string2);
	}

	if (t==1)
	{
		printf("My string [%d]: %p\n",t,string);
		s1=string2;
	}
	else if (t==2)
	{
		printf("My string [%d]: %p\n",t,string);
		//printf("Seg Fault?: %p\n",PHYS_MEMORY);
		s2=string2;
	}
	else if (t==3)
	{
		s3=string2;
	}
	else if (t==4)
	{
		s4=string2;
	}

        //free(string);
        //free(string2);
	pthread_yield();

	return NULL;
}
