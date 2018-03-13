#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_pthread_t.h"

#define malloc(x) myallocate((x), __FILE__, __LINE__, currentThread)
#define free(x) mydeallocate((x), __FILE__, __LINE__, 1)


//Function to allocate from static array
void* myallocate(size_t, char*, int, tcb*);

void mydeallocate(void*, char*, int, tcb*);
