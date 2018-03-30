#include "my_malloc.h"
#include "my_pthread_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
	int** myArr = (int**)myallocate(sizeof(int*)*5000, __FILE__, __LINE__, NULL);
	char* myArr2 = (char*)myallocate(sizeof(char)*2, __FILE__, __LINE__, NULL);
        int i = 0;
	myArr2[0] = 'u';
	myArr2[1] = '1';
	mydeallocate(myArr2,__FILE__,__LINE__,NULL);
	for(i=0;i<500;i++)
	{
		myArr[i] = (int*)myallocate(sizeof(int), __FILE__, __LINE__, NULL);
		*myArr[i] = i;
	}
	for(i=499;i>=0;i--)
	{
		mydeallocate(myArr[i],__FILE__,__LINE__,NULL);
	}
	mydeallocate(myArr,__FILE__,__LINE__,NULL);
	return 0;
}
