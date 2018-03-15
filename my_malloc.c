#include "my_malloc.h"
#include "my_pthread_t.h"

#define MEM_SIZE (1024 * 1024 * 8)
#define PAGE_SIZE (1024 * 4)
#define MAX_PAGES 256

/*
STATE YOUR ASSUMPTIONS:
  - left-most bit will be free-bit
  - 12 bits will be used for size
  - max allocation size will be 4kB (page size)
    + requires 12 bits


*/

unsigned char PHYS_MEMORY[MEM_SIZE];
int isSet; //check first malloc ever in physical memory
tcb *currentTCB;
short phys_index;
void *virtual_mem, *phys_mem;
short pageFlag;//0 = read operation, 1 = allocate write operation, 2 = deallocate translation, 3 = deallocate virtual memory

//Goes here on SEGFault
static void memory_manager(int signum, siginfo_t *si, void *ignoreMe)
{
  //assuming only threads will use virtual memory (user, not library)
  if(currentTCB == NULL)
  {return;}


  if(signum != SIGSEGV)
  {
    printf("NOOOO\n");
    exit(-1);
  }
  
  
  //read operation
  if(pageFlag == 0)
  {
    //address of signal trigger
    char *addr = (char*)si.si_addr;

    //TODO: account for offset here
    if(addr < currentTCB->page->pageTable)
    {
      //protect current page before swapping
      mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_NONE);

      //go backwards
      while(currentTCB->myPage != NULL)
      {
	if(addr >= currentTCB->myPage->pageTable && addr <= (currentTCB->myPage->pageTable + PAGE_SIZE))
        {
	  //page table found
	  mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_READ | PROT_WRITE);
	  break;
        }
	currentTCB->myPage = currentTCB->myPage->prevPage;
      }

      if(currentTCB->myPage == NULL)
      {
        printf("->Segmentation Fault\n");
	exit(-1);
      }
      pageFlag = 0;
      return;
    }

    //TODO: account for offset here
    else if(addr > (currentTCB->myPage->pageTable + PAGE_SIZE))
    {
      //protect current page before swapping
      mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_NONE);

      //go forwards
      while(currentTCB->myPage != NULL)
      {
	if(addr >= currentTCB->myPage->pageTable && addr <= (currentTCB->myPage->pageTable + PAGE_SIZE))
        {
	  //page table found
	  break;
        }

	currentTCB->myPage = currentTCB->myPage->nextPage;
      }

      if(currentTCB->myPage == NULL)
      {
        printf("->Segmentation Fault\n");
	exit(-1);
      }
    }

    //address translation
    int i = addr - currentTCB->myPage->pageTable; //TODO: account for offset here
    char* phys_addr = &PHYS_MEMORY[i];
    *si.si_addr = phys_addr; //TODO: FIX THIS MASSIVE SECURITY HOLE
    mprotect( buffer, pagesize, PROT_NONE);
    pageFlag = 0;
    return;
  }

  //write operation
  else
  {
    //allocate
    if(pageFlag == 1)
    {
      if(currentTCB->myPage->pageTable == NULL)
      {
        currentTCB->myPage->pageTable = (char*)memalign(PAGE_SIZE, (sizeof(char) * 4096);
        mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_READ | PROT_WRITE);
	bzero(currentTCB->myPage->pageTable, 4096);
	//memset(currentTCB->myPage->pageTable, '`', 4096);
	//TODO: account for offset here
	//setting null pointer at beginning
	currentTCB->myPage->pageTable[0] = 0;
	currentTCB->myPage->pageTable[1] = 0;
	currentTCB->myPage->pageTable[2] = 0;
	currentTCB->myPage->pageTable[3] = 0;
      }

      int pageIndex = 0;
      unsigned int meta = (PHYS_MEMORY[phys_index] << 16) | (PHYS_MEMORY[phys_index+1] << 8) | (PHYS_MEMORY[phys_index+2]); //metadata of block about to be allocated
      unsigned int mSize = (meta & 0x7fffff); //size of new allocation
      unsigned int vSize = mSize - sizeof(int*); //used to hold size of virtual memory block when searching
      currentTCB->myPage->alloc += mSize;
      char* virtual_ptr = NULL; //can't be char* because we need large enough block when dereferencing (CHECK THIS)
      

      while(pageIndex < 4096)
      {
        //TODO: array search

        //Step 1: pull pointer to phys addr from virt mem
	//TODO: account for offset here
        virtual_ptr = (currentTCB->myPage->pageTable[pageIndex] << 24) | (currentTCB->myPage->pageTable[pageIndex+1] << 16) |
        (currentTCB->myPage->pageTable[pageIndex+2] << 8) | currentTCB->myPage->pageTable[pageIndex+3];
      
        //Step 2: get metadata from physical memory
        if(virtual_ptr == 0) //checking if a pointer = 0 is the same as checking if it's null to see if it's free
        {
	  //virtual meta is 4 bytes; sizeof(pointer)
	  //Step 2.5: check if spot in VM is larger enough to hold allocation (of size mSize)
	  int p;
          int c = 0;
	  for(p = 0; p < mSize && c < PAGE_SIZE; p++)
	  {
	    if(currentTCB->myPage->pageTable[c] == '`')
	    {
	      p = -1;
	      continue;
	    }

	    if(currentTCB->myPage->pageTable[c] != 0)
	    {
	      //for now, assume this is the beginning of a pointer
	      virtual_ptr = (currentTCB->myPage->pageTable[c] << 24) | (currentTCB->myPage->pageTable[c+1] << 16) |
              (currentTCB->myPage->pageTable[c+2] << 8) | currentTCB->myPage->pageTable[c+3];
	  
	      int pIndex = virtual_ptr - PHYS_MEMORY;
	      unsigned int pMeta = (PHYS_MEMORY[pIndex] << 16) | (PHYS_MEMORY[pIndex+1] << 8) | (PHYS_MEMORY[pIndex+2]);
	      c += ((pMeta & 0x7fffff) - 1);
	      p = -1;
	      continue;
	    }
	    //TODO: account for page swap while searching
	    //mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_READ | PROT_WRITE);
	  }
        
	  //Step 3: writing physical address to pagetable
	  short insertIndex = c-mSize;
	  virtual_mem = &PHYS_MEMORY[phys_index] + (sizeof(char) * 3);
	  currentTCB->myPage->pageTable[insertIndex] = (virtual_mem >> 24) & 0xff;
	  currentTCB->myPage->pageTable[insertIndex+1] = (virtual_mem >> 16) & 0xff;
	  currentTCB->myPage->pageTable[insertIndex+2] = (virtual_mem >> 8) & 0xff;
	  currentTCB->myPage->pageTable[insertIndex+3] = virtual_mem & 0xff;
          mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_NONE);
	  pageFlag = 0;
	  return;
	}



    }

    //deallocate
    else
    {
      //Address translation
      if(pageFlag == 2)
      {
	if(virtual_mem < currentTCB->page->pageTable)
        {
          //go backwards
          while(currentTCB->myPage != NULL)
          {
	    if(virtual_mem >= currentTCB->myPage->pageTable && addr <= (currentTCB->myPage->pageTable + PAGE_SIZE))
            {
	      //page table found
	      mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_READ | PROT_WRITE);
	      break;
            }

	    currentTCB->myPage = currentTCB->myPage->prevPage;
          }

          if(currentTCB->myPage == NULL)
          {
            printf("->Segmentation Fault\n");
	    exit(-1);
          }

         }

        //TODO: account for offset here
        else if(virtual_mem > (currentTCB->myPage->pageTable + PAGE_SIZE))
        {
          //go forwards
          while(currentTCB->myPage != NULL)
          {
	    if(virtual_mem >= currentTCB->myPage->pageTable && addr <= (currentTCB->myPage->pageTable + PAGE_SIZE))
            {
	      //page table found
	      break;
            }

	    currentTCB->myPage = currentTCB->myPage->nextPage;
          }

          if(currentTCB->myPage == NULL)
          {
            printf("->Segmentation Fault\n");
	    exit(-1);
          }
        }

	int pIndex = virtual_mem - currentTCB->myPage->pageTable; //TODO: account for offset here
	phys_mem = (currentTCB->myPage->pageTable[pIndex] << 24) | (currentTCB->myPage->pageTable << 16) | (currentTCB->myPage->pageTable << 8) | (currentTCB->myPage->pageTable);
        mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_NONE);
	pageFlag = 0;
        return;
      }

      //Clear VM
      else if(pageFlag == 3)
      {
	mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_READ | PROT_WRITE);
	unsigned int pMeta = (phys_mem[0] << 16) | (phys_mem[1] << 8) | (phys_mem[2]);
	unsigned int size = pMeta & 0x7fffff;
	int pIndex = virtual_mem - currentTCB->myPage->pageTable; //TODO: account for offset here
	bzero(currentTCB->myPage->pageTable + pIndex, size); //zeroing VM
	mprotect(currentTCB->myPage->pageTable, PAGE_SIZE, PROT_NONE);
	pageFlag = 0;
	return;
      }

      //shouldn't ever happen
      else
      {
	printf("WHAT HAVE YOU DONE\n");
        exit(-1);
      }

    }

  pageFlag = 0;
  return;
}




void* myallocate(size_t size_req, char *fileName, int line, tcb* src)
{
  //TODO: search for free block: (assume first-fit)
  printf("Size Request: %zu\n",size_req);
  pageFlag = 1;
  int i = 0;
  unsigned int size_req_int = size_req;
  unsigned int meta; //2 bytes required to hold all metadata
  unsigned int isFree;
  unsigned int blockSize;
  int total_alloc;

  if(!isSet)
  {
    //fill in metadata
    int totalSize = MEM_SIZE - (sizeof(unsigned char) * 3);
    //(PHYS_MEMORY[i] >> 3) &= 0x1;
    totalSize &= 0x7fffff;
    unsigned char freeBit = 0x80; //1 means free?
    
    PHYS_MEMORY[i] = freeBit | ((totalSize >> 16) & 0x7f);
    PHYS_MEMORY[i+1] = (totalSize >> 8) & 0xff;
    PHYS_MEMORY[i+2] = totalSize & 0xff;

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = memory_manager;

    if(sigaction(SIGSEGV, &sa, NULL) == -1)
    {
      printf("Fatal error setting up signal handler\n");
      exit(EXIT_FAILURE);    //explode!
    }

    if(!mainRetrieved)
    {
      initializeMainContext();
    }

    //isSet = 1;
  }

  if(src == NULL)
  {
    //is a library thread
    total_alloc = -1;

    if(size_req_int > MEM_SIZE)
    {
      printf("Reurn NULL 1\n");
      return NULL;
    }
  }
  else
  {
    //is a user thread
    total_alloc = src->alloc + size_req_int + (sizeof(unsigned char) * 3);
  }

  if(total_alloc > PAGE_SIZE)
  {
    //block attempting to allocate more than max
    printf("Reurn NULL 2\n");
    return NULL;
  }

  while(i+2 < MEM_SIZE)
  {
    meta = (PHYS_MEMORY[i] << 16) | (PHYS_MEMORY[i+1] << 8) | (PHYS_MEMORY[i+2]);

    isFree = meta & 0x800000;
    blockSize = meta & 0x7fffff/*0x7ff8*/;


    if(isFree && blockSize >= size_req_int)
    {
      //valid block found
      printf("Block size currently is: %d\tMEM_SIZE was: %d\n",blockSize,MEM_SIZE);
      //note: 0 out header
      PHYS_MEMORY[i] &= 0x00;
      PHYS_MEMORY[i+1] &= 0x00;
      PHYS_MEMORY[i+2] &= 0x00;

      //for i: get leftmost 4 bits from size_req
      //for i + 1 0 out all except rightmost 8
      size_req_int &= 0x7fffff;
      printf("Registered request: %u\n",size_req_int);
      //1  1111111 | 11111111 | 11111111 
      //free bit then three blank bits then the size bits
      //so the above the is block is free and has a lot of memory
      PHYS_MEMORY[i] = (size_req_int >> 16) & 0x7f;//((isFree >> 16) & 0x0) | 
      PHYS_MEMORY[i+1] = (size_req_int >> 8) & 0xff;
      unsigned char temporary = (size_req_int & 0xff);
      PHYS_MEMORY[i+2] = temporary;

      //if there isn't enough data in the new block after splitting, then don't split at all
      if(blockSize - size_req_int - (sizeof(unsigned char) * 3) < 0)
      {
	printf("?\n");
      }
      else
      {
        int freeBit = 0x80;
        int remainingSize = blockSize - size_req_int - (sizeof(char)*3);
        int nextHeaderI = i+(sizeof(unsigned char) * 3)+size_req_int;

        PHYS_MEMORY[nextHeaderI] = freeBit | ((remainingSize >> 16) & 0x7f);
        PHYS_MEMORY[nextHeaderI+1] = (remainingSize >> 8) & 0xff;
        PHYS_MEMORY[nextHeaderI+2] = remainingSize & 0xff;
      }
      

      printf("Inserted request of %u at %p\n", size_req_int,&PHYS_MEMORY[i] + (sizeof(unsigned char) * 3));

      //if(!isSet)
      //{
      isSet = 1;
      currentTCB = src;
      phys_index = i;
      raise(SIGSEGV);
      //}
      if(src != NULL)
      {
        src->alloc += total_alloc;
      }

      return virtual_mem;
   }
  
    else
    {
      i += (blockSize + (sizeof(unsigned char) * 3));

    }

  }

  printf("NO VALID BLOCK\n");
  //if reached, no valid block	
  return NULL;
}


void mydeallocate(void *ptr, char *fileName, int line, tcb *src)
{
  unsigned char *location = NULL;

  if(src != NULL)
  {
    pageFlag = 2;
    raise(SIGSEGV);
    location = phys_mem;
    virtual_mem = (char*)ptr;
  }

  if(location == NULL)
  {
    location = (unsigned char*)ptr - (sizeof(unsigned char) * 3); //address of meta block of inputted block
  }

  int searchI = 0; //index of block in array

  printf("Deallocating %p\n", location);

  int index = (location - PHYS_MEMORY);
  int meta, isFree, prevBlockSize, blockSize, nextBlockSize, searchBSize, totalBlockSize = 0;//, firstBlockSize;
  int firstBlockIndex = index;

  if(index < 0 || index >= MEM_SIZE)
  {
    //fail case
    //TODO: can we just return here, or do we have to do some error checking?
    printf("Out of bounds error\tIndex: %d\n", index);
    return;
  }
  
  meta = (PHYS_MEMORY[index] << 16) | PHYS_MEMORY[index+1] << 8 | PHYS_MEMORY[index+2];
  
  isFree = meta & 0x800000;
  blockSize = meta & 0x7fffff;

  //checking current block
  if(isFree)
  {
    //TODO: handle double free
    printf("Attempted double free\n");
    return;
  }

  //check first element
  if(location == PHYS_MEMORY)
  {
    meta = (PHYS_MEMORY[0] << 16) | (PHYS_MEMORY[1] << 8) | (PHYS_MEMORY[2]);
    isFree = meta & 0x800000;
    prevBlockSize = meta & 0x7fffff;
    totalBlockSize += blockSize;
    firstBlockIndex = index;
  }
  else
  {
    while(location - &PHYS_MEMORY[searchI] != 0 && searchI < MEM_SIZE)
    {
      searchBSize = ((PHYS_MEMORY[searchI] << 16) | (PHYS_MEMORY[searchI+1] << 8) | (PHYS_MEMORY[searchI+2])) & 0x7fffff; //size of the previous block
      searchI += (searchBSize + (sizeof(unsigned char) * 3)); //address of current meta block we are on
    }

    //Probably won't happen, but just in case
    if(searchI > MEM_SIZE)
    {
      printf("That thing that shouldn't happen actually happened\n");
      return;
    }



    searchI -= (searchBSize + (sizeof(unsigned char) * 3));
    meta = (PHYS_MEMORY[searchI] << 16) | (PHYS_MEMORY[searchI+1] << 8) | (PHYS_MEMORY[searchI+2]);

    //Clear Virtual Memory
    if(src != NULL)
    {
      pageFlag = 3;
      raise(SIGSEGV);
    }

    isFree = meta & 0x800000;
    prevBlockSize = meta & 0x7fffff; //ignore the name, cuz technically this is the new block we're on

    //store current blocksize in total
    totalBlockSize += blockSize;

    //checks previous block
    if(isFree)
    {
      firstBlockIndex = searchI;
      totalBlockSize += (prevBlockSize + (sizeof(unsigned char) * 3));
    }

  }
  //check if last element
  if(index + blockSize + (sizeof(unsigned char) * 3) >= MEM_SIZE)
  {
    
  }
  else
  {
    searchI += blockSize + (sizeof(unsigned char) * 3);
    meta = (PHYS_MEMORY[searchI] << 16) | (PHYS_MEMORY[searchI+1] << 8) | (PHYS_MEMORY[searchI+2]);

    isFree = meta & 0x800000;
    nextBlockSize = meta & 0x7fffff; 
    if(isFree)
    {
      totalBlockSize += nextBlockSize + (sizeof(unsigned char) * 3);
    }
  }
  
  //Right check
  int right = index + blockSize + (sizeof(unsigned char) * 3);
  meta = (PHYS_MEMORY[right] << 16) | PHYS_MEMORY[right+1] << 8 | PHYS_MEMORY[right+2];


  printf("Total block size: %d\n", totalBlockSize);

  PHYS_MEMORY[firstBlockIndex] = 0x80 | ((totalBlockSize >> 16) & 0x7f);
  PHYS_MEMORY[firstBlockIndex+1] = (totalBlockSize >> 8) & 0xff;
  PHYS_MEMORY[firstBlockIndex+2] = (totalBlockSize & 0xff);


  //TODO: maybe, possibly, probably not zero out block
  
  if(src != NULL)
  {
    src->alloc -= blockSize;
  }
  printf("Success!\n");
}

