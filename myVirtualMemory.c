#include "my_pthread_t.h"

#define PHYS_MEM_S (8*1024*1024)
#define PAGE_S (4*1024)
#define THREADREQ 0
#define LIBRARYREQ 1
#define METADATA_S 0x1fffff

char PHYS_MEMORY[PHYS_MEM_S];

void *myallocate(size_t size,char* file, int line, tcb* src)
{
  int i = 0;
  unsigned short meta;
  int freebit;
  int blockSize;

  if(src==NULL)
  {
    //Library
  }
  else if(src->alloc + size_req > METADATA_S)
  {
    //Out of memory
    return NULL;
  }
  

  while (i < PHYS_MEM_S)
  {
    meta = PHYS_MEMORY[i] | PHYS_MEMORY[i+1];
    freebit = 0x800 & meta;
    blockSize = 

    if (freebit && blockSize >= size_req)
    {
      src->alooc += size_req;
      return (&PHYS_MEMORY[i] + sizeof(short));
    }
    else
    {
      i += (blockSize + sizeof(short));
    }
  }
}

void mydeallocate(void* ptr, char* file, int line, tcb* src)
{
}


00[1][1] 0000 | 0000 0111 |....|


    i=0           i=1           i=6
