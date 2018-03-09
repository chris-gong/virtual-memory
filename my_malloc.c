#include "my_malloc.h"
#in

#define MEM_SIZE (1024 * 1024 * 8)
#define MAX_MEM (1024 * 4)
#define METADATA_SIZE 0xc

/*
STATE YOUR ASSUMPTIONS:
  - left-most bit will be free-bit
  - 12 bits will be used for size
  - max allocation size will be 4kB (page size)
    + requires 12 bits


*/

char PHYS_MEMORY[MEM_SIZE];


void* myallocate(size_t size_req, char *fileName, int line, tcb src)
{
  //TODO: search for free block: (assume first-fit)
  int i = 0;
  unsigned short meta; //2 bytes required to hold all metadata
  int isFree;
  int blockSize;
  int total_alloc;

  if(src == NULL)
  {
    //is a library thread
    total_alloc = -1;
  }
  else
  {
    //is a user thread
    total_alloc = src->alloc + size_req + sizeof(short);
  }

  if(total_alloc > MAX_MEM)
  {
    //block attempting to allocate more than max
    return NULL;
  }

  while(i+1 < mem_limit)
  {
    meta = (PHYS_MEMORY[i] << 8) | PHYS_MEMORY[i+1];
    isFree = meta & 0x8000;
    blockSize = meta & 0x7ff8;

    if(isFree && blockSize >= size_req)
    {
      //valid block found

      //fill in metadata
      PHYS_MEMORY[i] = (isFree>>24) | ((blockSize>>24) & 0x7f);
      PHYS_MEMORY[i+1] = (blockSize>>16) & 0xff;

      if(src != NULL)
      {
        src->alloc += total_alloc;
      }

      return &(PHYS_MEMORY[i] + sizeof(short));
   }
  
    else
    {
      i += (blockSize + sizeof(short));

    }

  }

  //if reached, no valid block	
  return NULL;
}


void mydeallocate(void *ptr, char *fileName, int line, tcb src)
{
  (char*) block = ptr + 2;



}
