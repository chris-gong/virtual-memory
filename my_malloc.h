#include <stdio.h>
#include <stdlib.h>
#include <string.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <signal.h>
#include <math.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "my_pthread_t.h"

#define malloc(x) myallocate((x), __FILE__, __LINE__, 1) //1 for user
#define free(x) mydeallocate((x), __FILE__, __LINE__, 1) //1 for user


//Function to allocate from static array
void* myallocate(size_t, char*, int, char);
void mydeallocate(void*, char*, int, char);
void initializeSwapFile();
void swapMe(int, int, int);
void setMem();
int diskSearch(int);


//struct to hold: leftmost:freebit, pageNum, and Thread ID
typedef struct frameMeta
{
  char isFree;//if frame is available for use
  unsigned int owner;//thread ID
  unsigned int pageNum;
}frameMeta;
=======
=======
>>>>>>> 21fb54dd7fb473f5704d8e78cf372e08566da850
#include "my_pthread_t.h"

#define malloc(x) myallocate((x), __FILE__, __LINE__, currentThread)
#define free(x) mydeallocate((x), __FILE__, __LINE__, 1)


//Function to allocate from static array
void* myallocate(size_t, char*, int, tcb*);

void mydeallocate(void*, char*, int, tcb*);
<<<<<<< HEAD
>>>>>>> why god why
=======
>>>>>>> 21fb54dd7fb473f5704d8e78cf372e08566da850
