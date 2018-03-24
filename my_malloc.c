#include "my_malloc.h"
#include "my_pthread_t.h"

#define MEM_SIZE (1024 * 1024 * 8)
#define DISK_SIZE (1024 * 1024 * 16)
#define PAGE_SIZE (1024 * 4)
#define MAX_PAGES (1024 * 1024 * 8)/(1024 * 4)

/*
STATE YOUR ASSUMPTIONS:
  - left-most bit will be free-bit
  - 12 bits will be used for size
  - max allocation size will be 4kB (page size)
    + requires 12 bits
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

//Goes here on SEGFault
static void memory_manager(int signum, siginfo_t *si, void *ignoreMe)
{
  //assuming only threads will use virtual memory (user, not library)
  if(src == 0)
  {return;}


  if(signum != SIGSEGV)
  {
    printf("NOOOO\n");
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
{
  //TODO: search for free block: (assume first-fit)
  printf("Size Request: %zu\n",size_req);
  pageFlag = 1;
  int i = 0;
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

    isSet = 1;
  }

  request_size = size_req;




  if(src == 0)
  {
    //is a library thread
    //only difference here is that we dont do stuff for pages/frames
    if(size_req_int > (MEM_SIZE/2) - 3)
    {
      printf("Reurn NULL 1\n");
      return NULL;
    }
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
  }
  else
  {
    //is a user thread
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
  return NULL;
}


void mydeallocate(void *ptr, char *fileName, int line, char src)
{
  static unsigned char *location = NULL;


  if(location == NULL)
  {
    location = (static unsigned char*)ptr - (sizeof(static unsigned char) * 3); //address of meta block of inputted block
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
      searchI += (searchBSize + (sizeof(static unsigned char) * 3)); //address of current meta block we are on
    }

    //Probably won't happen, but just in case
    if(searchI > MEM_SIZE)
    {
      printf("That thing that shouldn't happen actually happened\n");
      return;
    }



    searchI -= (searchBSize + (sizeof(static unsigned char) * 3));
    meta = (PHYS_MEMORY[searchI] << 16) | (PHYS_MEMORY[searchI+1] << 8) | (PHYS_MEMORY[searchI+2]);


    isFree = meta & 0x800000;
    prevBlockSize = meta & 0x7fffff; //ignore the name, cuz technically this is the new block we're on

    //store current blocksize in total
    totalBlockSize += blockSize;

    //checks previous block
    if(isFree)
    {
      firstBlockIndex = searchI;
      totalBlockSize += (prevBlockSize + (sizeof(static unsigned char) * 3));
    }

  }
  //check if last element
  if(index + blockSize + (sizeof(static unsigned char) * 3) >= MEM_SIZE)
  {
    
  }
  else
  {
    searchI += blockSize + (sizeof(static unsigned char) * 3);
    meta = (PHYS_MEMORY[searchI] << 16) | (PHYS_MEMORY[searchI+1] << 8) | (PHYS_MEMORY[searchI+2]);

    isFree = meta & 0x800000;
    nextBlockSize = meta & 0x7fffff; 
    if(isFree)
    {
      totalBlockSize += nextBlockSize + (sizeof(static unsigned char) * 3);
    }
  }
  
  //Right check
  int right = index + blockSize + (sizeof(static unsigned char) * 3);
  meta = (PHYS_MEMORY[right] << 16) | PHYS_MEMORY[right+1] << 8 | PHYS_MEMORY[right+2];


  printf("Total block size: %d\n", totalBlockSize);

  PHYS_MEMORY[firstBlockIndex] = 0x80 | ((totalBlockSize >> 16) & 0x7f);
  PHYS_MEMORY[firstBlockIndex+1] = (totalBlockSize >> 8) & 0xff;
  PHYS_MEMORY[firstBlockIndex+2] = (totalBlockSize & 0xff);

  printf("Success!\n");
}

