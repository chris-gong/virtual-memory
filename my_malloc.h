#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define malloc(x) myallocate((x), __FILE__, __LINE__, currentThread)
#define free(x) mydeallocate((x), __FILE__, __LINE__, 1)


//Function to allocate from static array
void* myallocate(size_t, char*, int, int);

void mydeallocate(void*, char*, int, int);
