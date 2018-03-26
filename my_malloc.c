#include "my_malloc.h"
#include "my_pthread_t.h"

#define MEM_SIZE (1024 * 1024 * 8)
<<<<<<< HEAD
#define DISK_SIZE (1024 * 1024 * 16)
#define PAGE_SIZE (1024 * 4)
#define MAX_PAGES (1024 * 1024 * 8)/(1024 * 4)
=======
#define PAGE_SIZE (1024 * 4)
#define MAX_PAGES 256
>>>>>>> why god why

/*
STATE YOUR ASSUMPTIONS:
  - left-most bit will be free-bit
  - 12 bits will be used for size
  - max allocation size will be 4kB (page size)
    + requires 12 bits
<<<<<<< HEAD
  -max allocation per thread will be 4MB - 3
*/

static unsigned char * PHYS_MEMORY;
static frameMeta frameMetaPhys[(MEM_SIZE/PAGE_SIZE)/2];
static frameMeta frameMetaDisk[(DISK_SIZE/PAGE_SIZE)];


int isSet; //check first malloc ever in physical memory
tcb *currentTCB;
short phys_index;
unsigned int size_req_int;
int firstPage;
char *startAddr; //used for signal handler to return to malloc in the case where more memory is requested but can be fulfilled by swapping in pages, corresponds with lastBlockSize TODO: set in malloc
short pageFlag;//0 = invalid read/write from thread, 1 = malloc when creating a new header on a split on a different page, malloc write. 2=free when looking for blocks to the sides (for purpose of coalescing)
size_t request_size; //globalize size request to pass to signal handler
int lastBlockSize; //only set when we find a free block that can't fulfill the size of request, but needs to be used to determine how many pages to swap in
                   //think about the case where we are in page 1 and there's a little block at the end of it that's free, but the next page is owned by another thread
=======


*/

unsigned char PHYS_MEMORY[MEM_SIZE];
int isSet; //check first malloc ever in physical memory
tcb *currentTCB;
short phys_index;
void *virtual_mem, *phys_mem;
short pageFlag;//0 = read operation, 1 = allocate write operation, 2 = deallocate translation, 3 = deallocate virtual memory
>>>>>>> why god why

//Goes here on SEGFault
static void memory_manager(int signum, siginfo_t *si, void *ignoreMe)
{
  //assuming only threads will use virtual memory (user, not library)
<<<<<<< HEAD
  if(src == 0)
=======
  if(currentTCB == NULL)
>>>>>>> why god why
  {return;}


  if(signum != SIGSEGV)
  {
    printf("NOOOO\n");
<<<<<<< HEAD
    exit(EXIT_FAILURE);
  }

  uintptr_t src_page = (uintptr_t)si->si_addr;
  int src_pageNum = (page >> 12) & 0xfffff;
  int src_offset = page & 0xfff;
  if(firstPage == -1)
  {
    uintptr_t startFrame = (uintptr_t)&PHYS_MEMORY[MEM_SIZE/2];
    firstPage = (startFrame >> 12) & 0xfffff;
  }

  pageNum -= firstPage;

  if(pageFlag == 0) //thread accessing invalid memory
  {
    printf("->Segmentation Fault");
    exit(EXIT_FAILURE);
  }

  else if(pageFlag == 1) //malloc on splitting
  {
    if(startAddr == NULL)
    {
      printf("Error in malloc; page searching");
      exit(EXIT_FAILURE);
    }

    uintptr_t page = (uintptr_t)startAddr;
    int pageNum = (page >> 12) & 0xfffff;
    int offset = page & 0xfff;

    pageNum -= firstPage;

    int page_request = ceil((double)(size_req_int)/ PAGE_SIZE);  //size_req_int is a global variable, should be spaceLeft in malloc
    int metaIndex, diskIndex, startIndex; //size, meta, starting location for loop
    int s;//reperesents meta
    int currTID = 0x0;
    int diskIndex = 0;
    char freeBit = 0x0;
    
    /*Check for if all pages are able to be used*/
    /*for(s = 0; m < page_request; s++)
    {
      //check if we're outside of physical memory
      if(src_pageNum+s >= MEM_SIZE/PAGE_SIZE)
      {
        printf("Ran out of physical memory\n");
        startAddr = NULL;
        return;
      }
    }*/

    /*Filling meta data for this request*/
    for(s = 0; s < page_request; s++)
    {
      //search for s amount of free frames to swap in
      //TODO: MAKE THIS PART FASTER
      //check if page we are on belongs to current thread, if not then we need to check if thread has this page in disk so that we can create one if necessary
      freeBit = (frameMetaPhys[src_pageNum + s].m1 >> 7);
      currTid = (frameMetaPhys[src_pageNum + s].m1 >> 1) & 0x3f;
      if (s == 0)
      {
        //unprotect
	/*frameMetaPhys[pageNum + s].m1 = 0x3f;//state no longer free, pass in thread ID
	frameMetaPhys[pageNum +s].m2 = 
        mprotect(&PHYS_MEMORY[s], PAGE_SIZE, PROT_READ | PROT_WRITE);*/
	diskIndex = diskSearchMallocWrite(pageNum+s,diskIndex);
      }
      else if (currentThread->tid != currTid || freeBit)
      {
        
      }
    }
  }

  else if(pageFlag == 2) //TODO: what to do if we have an entirely freed page?
  {

  }  

  return;
}



int diskSearchMallocWrite(int metaPage,int start)
{
  int i=start;
  int diskSize = DISK_SIZE/PAGE_SIZE;
  char freeBit;
  int actualI = 0;
  int pageID;

  uintptr_t page = (uintptr_t)startAddr;
  int offset = page & 0xfff;

  //search disk for existing page first
  for(i = start; i < diskSize; i++)
  {
    disk_pageID = ((frameMetaDisk[i].m2 & 0x7) << 8) | (frameMetaDisk[i].m3 & 0xff);
    phys_pageID = meta & 0x7ff;
    freeBit = (frameMetaDisk[i].m1 >> 7);

    if(!freeBit && disk_pageID == phys_pageID)
    {
      actualI = i * PAGE_SIZE;
      DISK[actualI + offset] = freeBit | ((size_req_int >> 16) & 0x7f); //size_req_int is a global variable, specifically spaceLeft in malloc
      DISK[actualI+1 + offset] = (size_req_int >> 8) & 0xff;
      DISK[actualI+2 + offset] = size_req_int & 0xff;
      return i;
    }
  }

  //search disk for free page after
  for (i=start; i < diskSize; i++)
  {
    freeBit = (frameMetaDisk[i].m1 >> 7);

    if (isFree)
    {
      frameMetaDisk[i].m1 = (currentThread->tid & 0x3f) << 1;
      frameMetaDisk[i].m2 = (metaPage >> 8)& 0x7;
      frameMetaDisk[i].m3 = metaPage & 0xff;
                               
      actualI = i * PAGE_SIZE;
      DISK[actualI + offset] = freeBit | ((size_req_int >> 16) & 0x7f); //size_req_int is a global variable, specifically spaceLeft in malloc
      DISK[actualI+1 + offset] = (size_req_int >> 8) & 0xff;
      DISK[actualI+2 + offset] = size_req_int & 0xff;
      break;
    }
  }

  return i;
}

int diskSearchCreatePage(int metaPage, int start)
{

}

void* myallocate(size_t size_req, char *fileName, int line, char src)
=======
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
>>>>>>> why god why
{
  //TODO: search for free block: (assume first-fit)
  printf("Size Request: %zu\n",size_req);
  pageFlag = 1;
  int i = 0;
<<<<<<< HEAD
  size_req_int = size_req;
  unsigned int meta; //2 bytes required to hold all metadata
  unsigned int isFree;
  unsigned int blockSize;
  int total_alloc;
  int start_index;
  int bound;

  if (size_req_int <= 0)
  {
    return NULL;
  }

  if(!isSet)
  {
    pageFlag = 0;

    //if malloc was never called then that means we never memaligned 8MB or however much we want to
    //memalign for disk and physical memory
    PHYS_MEMORY = (static unsigned char*)memalign(PAGE_SIZE, MEM_SIZE);
    //make 8MB/4KB number or however many we want number of frames
    
    short total_num_pages = (MEM_SIZE/PAGE_SIZE); //only use frames for thread side of mem
    short i;
    for(i = (total_num_pages/2); i < total_num_pages; i++)
    {
      //set up frame headers for every PAGE_SIZE bytes of frameMetaPhys
      //metadata for frame: leftmost bit for free bit, next 6 bits for TID, rightmost 11 bits for page index number (0 .. MEM_SIZE/PAGE_SIZE)
      // [0][000 000]0 | 0000 0[000 | 0000 0000]
      short page_index = i-(total_num_pages/2);
      frameMetaPhys[page_index].m1 = 0x80;
      frameMetaPhys[page_index].m2 = (page_index >> 8) & 0x7;
      frameMetaPhys[page_index].m3 = page_index & 0xff;
    }
    total_num_pages = DISK_SIZE/PAGE_SIZE;freeBit && blockSize >= size_req_int
    for(i = 0; i < total_num_pages; i++)
    {
      frameMetaDisk[i].m1 = 0x80;
      frameMetaDisk[i].m2 = 0x00;
      frameMetaDisk[i].m3 = 0x00;
    }
    //set up two blocks of size 4 MB, first half is user memory, second half is OS/library memory
    //create blocks of size MEM_SIZE/2
    unsigned char* curr = PHYS_MEMORY;
    unsigned char freeBit = 0x80;
    int pIndex = curr - PHYS_MEMORY;
    int totalSize = MEM_SIZE/2 - 3; //-3 because header is 3 bytes

    //Allocation metadata: leftmost bit for free bit, 23 bits for allocation size
    //first the OS memory
    PHYS_MEMORY[pIndex] = freeBit | ((totalSize >> 16) & 0x7f);
    PHYS_MEMORY[pIndex+1] = (totalSize >> 8) & 0xff;
    PHYS_MEMORY[pIndex+2] = totalSize & 0xff;

    //second the user memory
    pIndex += MEM_SIZE/2;
    PHYS_MEMORY[pIndex] = freeBit | ((totalSize >> 16) & 0x7f);
    PHYS_MEMORY[pIndex+1] = (totalSize >> 8) & 0xff;
    PHYS_MEMORY[pIndex+2] = totalSize & 0xff;
    
    //mprotect entire memory
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_NONE);

=======
  unsigned int size_req_int = size_req;
  unsigned int meta; //2 bytes required to hold all metadata
  unsigned int isFree;
  unsigned int blockSize;
  unsigned int remaining;
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
>>>>>>> why god why

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

<<<<<<< HEAD
    isSet = 1;
  }

  request_size = size_req;




  if(src == 0)
  {
    //is a library thread
    //only difference here is that we dont do stuff for pages/frames
    if(size_req_int > (MEM_SIZE/2) - 3)
=======
    //isSet = 1;
  }

  if(src == NULL)
  {
    //is a library thread
    total_alloc = -1;

    if(size_req_int > MEM_SIZE)
>>>>>>> why god why
    {
      printf("Reurn NULL 1\n");
      return NULL;
    }
<<<<<<< HEAD
    //allocate memory in 1st half of PHYS_MEM
    start_index = 0;
    bound = MEM_SIZE/2;
    while(start_index < bound)
    {
      //check if block is free
      //and if it is free then check if it's big enough to hold the memory
      //first bit is free bit, next 23 bits are the size bits
      meta = (PHYS_MEMORY[start_index] << 16) | (PHYS_MEMORY[start_index+1] << 8) | (PHYS_MEMORY[start_index+2]);

      isFree = (meta >> 23) & 0x1;
      blockSize = meta & 0x7fffff/*0x7ff8*/;
      if(freeBit && blockSize >= size_req_int)
      {
        //rewrite header data of block and split if necessary
        PHYS_MEMORY[start_index] = (size_req_int >> 16) & 0x7f;
        PHYS_MEMORY[start_index+1] = (size_req_int >> 8) & 0xff;
        PHYS_MEMORY[start_index+2] = (size_req_int & 0xff);
        void *addrToReturn = &PHYS_MEMORY[start_index];
        if(blockSize - (sizeof(char) * 3) - size_req_int > 0)
        {
          //split if only new block created has enough room to fit a header and at least one byte of data
          start_index += size_req_int + (sizeof(char) * 3);
          unsigned int spaceLeft = blockSize - (sizeof(char) * 3) - size_req_int;
          spaceLeft |= 0x800000;
          PHYS_MEMORY[start_index] = (spaceLeft >> 16) & 0xff;
          PHYS_MEMORY[start_index+1] = (spaceLeft >> 8) & 0xff;
          PHYS_MEMORY[start_index+2] = (spaceLeft & 0xff);
        }
        return addrToReturn;
      }
      else
      {
        start_index += (sizeof(char) * 3) + blockSize;
      }
    }
    printf("Exceeded OS memory capacity\n");
    return NULL;
=======
>>>>>>> why god why
  }
  else
  {
    //is a user thread
<<<<<<< HEAD
    //assume at this point, we just returned from the signal handler and 
    //the context switch replaced the last thread's pages with the new ones
    if(size_req_int > (MEM_SIZE/2) - 3)
    {
      printf("Reurn NULL 1\n");
      return NULL;
    }
    //allocate memory in 2nd half of PHYS_MEM
    start_index = MEM_SIZE/2;
    bound = MEM_SIZE;
    while(start_index < bound)
    {
/*
      //first check if current block we are on belongs to the current thread or is a free page
      unsigned int pNum = (bound - start_index)/PAGE_SIZE;
      unsigned int meta = (frameMetaPhys[pNum].m1 << 16) | (frameMetaPhys[pNum].m2 << 8) | (frameMetaPhys[pNum].m3);
      isFree = (meta >> 23) & 0x1;
      unsigned int threadId = (meta >> 17) & 0x3f;
      unsigned int pageIndex = (meta & 0x7ff); //get the right most 11 bits
*/
      //check if block is free
      //and if it is free then check if it's big enough to hold the memory
      //first bit is free bit, next 23 bits are the size bits
      meta = (PHYS_MEMORY[start_index] << 16) | (PHYS_MEMORY[start_index+1] << 8) | (PHYS_MEMORY[start_index+2]);

      isFree = (meta >> 23) & 0x1;
      blockSize = meta & 0x7fffff/*0x7ff8*/;
      if(freeBit && blockSize >= size_req_int)
      {
        //rewrite header data of block and split if necessary
        PHYS_MEMORY[start_index] = (size_req_int >> 16) & 0x7f;
        PHYS_MEMORY[start_index+1] = (size_req_int >> 8) & 0xff;
        PHYS_MEMORY[start_index+2] = (size_req_int & 0xff);
        void *addrToReturn = &PHYS_MEMORY[start_index];
        if(blockSize - (sizeof(char) * 3) - size_req_int > 0)
        {
          //split if only new block created has enough room to fit a header and at least one byte of data
          start_index += size_req_int + (sizeof(char) * 3);
          unsigned int spaceLeft = blockSize - (sizeof(char) * 3) - size_req_int;
          spaceLeft |= 0x800000;
          PHYS_MEMORY[start_index] = (spaceLeft >> 16) & 0xff;
          PHYS_MEMORY[start_index+1] = (spaceLeft >> 8) & 0xff;
          PHYS_MEMORY[start_index+2] = (spaceLeft & 0xff);
        }
        return addrToReturn;
      }
      else
      {
        start_index += (sizeof(char) * 3) + blockSize;
      }
    }
  }

  printf("NO VALID BLOCK\n");
  //if reached, no valid block	
=======
    total_alloc = src->alloc + size_req_int + (sizeof(unsigned char) * 3);
  }

  /*if(total_alloc > PAGE_SIZE)
  {
    //block attempting to allocate more than max
    printf("Reurn NULL 2\n");
    return NULL;
  }*/
  
    while(i+2 < PAGE_SIZE)
    {
      meta = (PHYS_MEMORY[i] << 16) | (PHYS_MEMORY[i+1] << 8) | (PHYS_MEMORY[i+2]);

      isFree = meta & 0x800000;
      blockSize = meta & 0x7fffff/*0x7ff8*/;
      remaining = PAGE_SIZE-i;


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
     else if (isFree && remaining < size_req_int && blockSize == size_req_int)
     {
       //Page swap
     }
     else
     {
       i += (blockSize + (sizeof(unsigned char) * 3));
     }
   }

   printf("NO VALID BLOCK\n");
   //if reached, no valid block	
>>>>>>> why god why
  return NULL;
}


<<<<<<< HEAD
void mydeallocate(void *ptr, char *fileName, int line, char src)
{
  static unsigned char *location = NULL;


  if(location == NULL)
  {
    location = (static unsigned char*)ptr - (sizeof(static unsigned char) * 3); //address of meta block of inputted block
=======
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
>>>>>>> why god why
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
<<<<<<< HEAD
      searchI += (searchBSize + (sizeof(static unsigned char) * 3)); //address of current meta block we are on
=======
      searchI += (searchBSize + (sizeof(unsigned char) * 3)); //address of current meta block we are on
>>>>>>> why god why
    }

    //Probably won't happen, but just in case
    if(searchI > MEM_SIZE)
    {
      printf("That thing that shouldn't happen actually happened\n");
      return;
    }



<<<<<<< HEAD
    searchI -= (searchBSize + (sizeof(static unsigned char) * 3));
    meta = (PHYS_MEMORY[searchI] << 16) | (PHYS_MEMORY[searchI+1] << 8) | (PHYS_MEMORY[searchI+2]);

=======
    searchI -= (searchBSize + (sizeof(unsigned char) * 3));
    meta = (PHYS_MEMORY[searchI] << 16) | (PHYS_MEMORY[searchI+1] << 8) | (PHYS_MEMORY[searchI+2]);

    //Clear Virtual Memory
    if(src != NULL)
    {
      pageFlag = 3;
      raise(SIGSEGV);
    }
>>>>>>> why god why

    isFree = meta & 0x800000;
    prevBlockSize = meta & 0x7fffff; //ignore the name, cuz technically this is the new block we're on

    //store current blocksize in total
    totalBlockSize += blockSize;

    //checks previous block
    if(isFree)
    {
      firstBlockIndex = searchI;
<<<<<<< HEAD
      totalBlockSize += (prevBlockSize + (sizeof(static unsigned char) * 3));
=======
      totalBlockSize += (prevBlockSize + (sizeof(unsigned char) * 3));
>>>>>>> why god why
    }

  }
  //check if last element
<<<<<<< HEAD
  if(index + blockSize + (sizeof(static unsigned char) * 3) >= MEM_SIZE)
=======
  if(index + blockSize + (sizeof(unsigned char) * 3) >= MEM_SIZE)
>>>>>>> why god why
  {
    
  }
  else
  {
<<<<<<< HEAD
    searchI += blockSize + (sizeof(static unsigned char) * 3);
=======
    searchI += blockSize + (sizeof(unsigned char) * 3);
>>>>>>> why god why
    meta = (PHYS_MEMORY[searchI] << 16) | (PHYS_MEMORY[searchI+1] << 8) | (PHYS_MEMORY[searchI+2]);

    isFree = meta & 0x800000;
    nextBlockSize = meta & 0x7fffff; 
    if(isFree)
    {
<<<<<<< HEAD
      totalBlockSize += nextBlockSize + (sizeof(static unsigned char) * 3);
=======
      totalBlockSize += nextBlockSize + (sizeof(unsigned char) * 3);
>>>>>>> why god why
    }
  }
  
  //Right check
<<<<<<< HEAD
  int right = index + blockSize + (sizeof(static unsigned char) * 3);
=======
  int right = index + blockSize + (sizeof(unsigned char) * 3);
>>>>>>> why god why
  meta = (PHYS_MEMORY[right] << 16) | PHYS_MEMORY[right+1] << 8 | PHYS_MEMORY[right+2];


  printf("Total block size: %d\n", totalBlockSize);

  PHYS_MEMORY[firstBlockIndex] = 0x80 | ((totalBlockSize >> 16) & 0x7f);
  PHYS_MEMORY[firstBlockIndex+1] = (totalBlockSize >> 8) & 0xff;
  PHYS_MEMORY[firstBlockIndex+2] = (totalBlockSize & 0xff);

<<<<<<< HEAD
  printf("Success!\n");
}

=======

  //TODO: maybe, possibly, probably not zero out block
  
  if(src != NULL)
  {
    src->alloc -= blockSize;
  }
  printf("Success!\n");
}


>>>>>>> why god why
