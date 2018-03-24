// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Lance Fletcher, Jeremy Banks, Christopher Gong
// username of iLab: laf224
// iLab Server: composite.cs.rutgers.edu

#include "my_pthread_t.h"
#include "my_malloc.h"

#define STACK_S (8 * 1024) //8kB stack frames
#define READY 0
#define YIELD 1
#define WAIT 2
#define EXIT 3
#define JOIN 4
#define MUTEX_WAIT 5
#define MAX_SIZE 15
#define INTERVAL 20000

//-----------------------------
#define MEM_SIZE (1024 * 1024 * 8)
#define DISK_SIZE (1024 * 1024 * 16)
#define PAGE_SIZE (1024 * 4)
#define MAX_PAGES (1024 * 1024 * 8)/(1024 * 4)
#define MEM_SECTION (MEM_SIZE/2)
#define META_SWAP_S (DISK_SIZE/PAGE_SIZE)
#define META_PHYS_S ((MEM_SIZE/PAGE_SIZE)/2)
#define CLEAR_FLAG 0
#define CREATE_PAGE 1
#define CONTEXT_SWITCH 2
#define EXTEND_PAGES 3
#define FREE_FRAMES 4
//-----------------------------


tcb *currentThread, *prevThread;
list *runningQueue[MAX_SIZE];
list *allThreads[MAX_SIZE];
ucontext_t cleanup;
sigset_t signal_set;
mySig sig;

//------------------------------------------------------
/*MEMORY GLOBALS*/
static unsigned char * PHYS_MEMORY;//Physical Memory (obviously)
static frameMeta frameMetaPhys[META_PHYS_S];//metadata for physical memory
static frameMeta frameMetaSwap[META_SWAP_S];//metadata for swap file
short pageFlag;//0 = no error, 1 = malloc-request, 2 free coalescing
size_t request_size;//requested size from malloc
char* startAddr;//on a malloc, address of a already allocated header, but we need to now actually make all its frame metas
FILE* swapfile;//disk space to hold page frames
int isSet;//flag to initialize memory, disk, and metadata
int swapFileFD;//Swap file file descriptor
int blockToFreeSize;//used in the signal handler to find out how many pages we need to free (frame meta wise)


//-----------------------------------------------------

struct itimerval timer, currentTime;

int mainRetrieved;
int timeElapsed;
int threadCount;
int notFinished;


//L: Signal handler to reschedule upon VIRTUAL ALARM signal
void scheduler(int signum)
{
  if(notFinished)
  {
    //printf("caught in the handler! Get back!\n");
    return;
  }


  //Record remaining time
  getitimer(ITIMER_VIRTUAL, &currentTime);


  //AT THIS POINT THE CURRENT THREAD IS NOT IN THE SCHEDULER (running queue, but it's always in allthreads)
  //once the timer finishes, the value of it_value.tv_usec will reset to the interval time (note this was when we set it_interval only)
  //printf("\n[Thread %d] Signaled from %d, time left %i\n", currentThread->tid,currentThread->tid, (int)currentTime.it_value.tv_usec);

  //L: disable timer if still activehttps://www.google.com/search?q=complete+v
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  setitimer(ITIMER_VIRTUAL, &timer, NULL);

  if(signum != SIGVTALRM)
  {
    /*TODO: PANIC*/
    //printf("[Thread %d] Signal Received: %d.\nExiting...\n", currentThread->tid, signum);
    exit(signum);
  }

  //TODO: mprotect here (implement after porting malloc.c here)


  //L: Time elapsed = difference between max interval size and time remaining in timer
  //if the time splice runs to completion the else body goes,
  //else the if body goes, and the actual amount of time that passed is added to timeelapsed
  int timeSpent = (int)currentTime.it_value.tv_usec;
  int expectedInterval = INTERVAL * (currentThread->priority + 1);
  //printf("timeSpent: %i, expectedInterval: %i\n", timeSpent, expectedInterval);
  if(timeSpent < 0 || timeSpent > expectedInterval)
  {
    timeSpent = 0;
  }
  else
  {
    timeSpent = expectedInterval - timeSpent;
  }

  
  timeElapsed += timeSpent;
  //printf("total time spend so far before maintenance cycle %i and the amount of time spent just now %i\n", timeElapsed, timeSpent);
  //printf("[Thread %d] Total time: %d from time remaining: %d out of %d\n", currentThread->tid, timeElapsed, (int)currentTime.it_value.tv_usec, INTERVAL * (currentThread->priority + 1));

  //L: check for maintenance cycle
  if(timeElapsed >= 10000000)
  {
    //printf("\n[Thread %d] MAINTENANCE TRIGGERED\n\n",currentThread->tid);
    maintenance();

    //L: reset counter
    timeElapsed = 0;
  }

  prevThread = currentThread;
  
  int i;

  switch(currentThread->status)
  {
    case READY: //READY signifies that the current thread is in the running queue

      if(currentThread->priority < MAX_SIZE - 1)
      {
	currentThread->priority++;
      }

      //put back the thread that just finished back into the running queue
      enqueue(&runningQueue[currentThread->priority], currentThread);

      currentThread = NULL;

      for(i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          //getting a new thread to run
          currentThread = dequeue(&runningQueue[i]);
	  break;
        }
	else
	{
	}
      }

      if(currentThread == NULL)
      {
        currentThread = prevThread;
      }

      break;
   
    case YIELD: //YIELD signifies pthread yield was called; don't update priority

      currentThread = NULL;

      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(&runningQueue[i]);
	  break;
        }
      }
      
      if(currentThread != NULL)
      {
	//later consider enqueuing it to the waiting queue instead
	enqueue(&runningQueue[prevThread->priority], prevThread);
      }
      else
      {
	currentThread = prevThread;
      }

      break;

    case WAIT:
      //L: When would something go to waiting queue?
      //A: In the case of blocking I/O, how do we detect this? Sockets
      //L: GG NOT OUR PROBLEM ANYMORE
      //enqueue(&waitingQueue, currentThread);
      
      break;

    case EXIT:

      currentThread = NULL;

      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        {
          currentThread = dequeue(&runningQueue[i]);
	  break;
        }
      }

      if(currentThread == NULL)
      {
	//L: what if other threads exist but none are in running queue?
	//printf("No other threads found. Exiting\n");

	//L: DO NOT USE EXIT() HERE. THAT IS A LEGIT TIME BOMB. ONLY USE RETURN
        return;
      }
      //L: free the thread control block and ucontext
      mydeallocate(prevThread->context->uc_stack.ss_sp, __FILE__, __LINE__, NULL);
      mydeallocate(prevThread->context, __FILE__, __LINE__, NULL);
      mydeallocate(prevThread, __FILE__, __LINE__, NULL);

      currentThread->status = READY;

      //printf("Switching to: TID %d Priority %d\n", currentThread->tid, currentThread->priority);

      //L: reset timer
      timer.it_value.tv_sec = 0;
      timer.it_value.tv_usec = INTERVAL * (currentThread->priority + 1);
      timer.it_interval.tv_sec = 0;
      timer.it_interval.tv_usec = 0;
      int ret = setitimer(ITIMER_VIRTUAL, &timer, NULL);
      if (ret < 0)
      {
        //printf("Timer Reset Failed. Exiting...\n");
        exit(0);
      }
      setcontext(currentThread->context);

      break;

    case JOIN: //JOIN corresponds with a call to pthread_join

      currentThread = NULL;
      //notice how we don't enqueue the thread that just finished back into the running queue
      //we just go straight to getting another thread
      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(&runningQueue[i]);
	  break;
        }
      }

      if(currentThread == NULL)
      {
	/*WE'VE GOT A PROBLEM*/
	exit(EXIT_FAILURE);
      }
      
      break;
      
    case MUTEX_WAIT: //MUTEX_WAIT corresponds with a thread waiting for a mutex lock

      //L: Don't add current to queue: already in mutex queue
      currentThread = NULL;

      for (i = 0; i < MAX_SIZE; i++) 
      {
        if (runningQueue[i] != NULL)
        { 
          currentThread = dequeue(&runningQueue[i]);
	  break;
        }
      }

      if(currentThread == NULL)
      {
        /*OH SHIT DEADLOCK*/
        //printf("DEADLOCK DETECTED\n");
	exit(EXIT_FAILURE);
      }

      break;

    default:
      //printf("Thread Status Error: %d\n", currentThread->status);
      exit(-1);
      break;
  }

	
  currentThread->status = READY;

  //L: reset timer to 25ms times thread priority
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = INTERVAL * (currentThread->priority + 1) ;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  int ret = setitimer(ITIMER_VIRTUAL, &timer, NULL);

  if (ret < 0)
  {
     //printf("Timer Reset Failure. Exiting...\n");
     exit(0);
  }

  //printf("Switching to: TID %d Priority %d\n", currentThread->tid, currentThread->priority);
  //Switch to new context
  if(prevThread->tid == currentThread->tid)  
  {/*Assume switching to same context is bad. So don't do it.*/}
  else
  {
    pageFlag = CONTEXT_SWITCH;
    raise(SIGSEGV);
    pageFlag = CLEAR_FLAG;
    swapcontext(prevThread->context, currentThread->context);
  }

  return;
}

//L: thread priority boosting
void maintenance()
{
  int i;
  tcb *tgt;


  //L: template for priority inversion
  for(i = 1; i < MAX_SIZE; i++)
  {
    while(runningQueue[i] != NULL)
    {
      tgt = dequeue(&runningQueue[i]);
      tgt->priority = 0;
      enqueue(&runningQueue[0], tgt);
    }
  }

  return;
}

//L: handle exiting thread: supports invoked/non-invoked pthread_exit call
void garbage_collection()
{
  //L: Block signal here

  notFinished = 1;

  currentThread->status = EXIT;
  
  //if we havent called pthread create yet
  if(!mainRetrieved)
  {
    exit(EXIT_SUCCESS);
  }

  tcb *jThread = NULL; //any threads waiting on the one being garbage collected

  //L: dequeue all threads waiting on this one to finish
  while(currentThread->joinQueue != NULL)
  {
    jThread = l_remove(&currentThread->joinQueue);
    jThread->retVal = currentThread->jVal;
    enqueue(&runningQueue[jThread->priority], jThread);
  }

  //L: free stored node in allThreads
  int key = currentThread->tid % MAX_SIZE;
  if(allThreads[key]->thread->tid == currentThread->tid)
  {
    list *removal = allThreads[key];
    allThreads[key] = allThreads[key]->next;
    mydeallocate(removal, __FILE__, __LINE__, NULL); 
  }

  else
  {
    list *temp = allThreads[key];
    while(allThreads[key]->next != NULL)
    {
      if(allThreads[key]->next->thread->tid == currentThread->tid)
      {
	list *removal = allThreads[key]->next;
	allThreads[key]->next = removal->next;
	mydeallocate(removal, __FILE__, __LINE__, NULL);
        break;
      }
      allThreads[key] = allThreads[key]->next;
    }

    allThreads[key] = temp;
  }

  notFinished = 0;

  raise(SIGVTALRM);
}

//L: add to queue
void enqueue(list** q, tcb* insert)
{
  list *queue = *q;

  if(queue == NULL)
  {
    queue = (list*)myallocate(sizeof(list), __FILE__, __LINE__, NULL);
    queue->thread = insert;
    queue->next = queue;
    *q = queue;
    return;
  }

  list *front = queue->next;
  queue->next = (list*)myallocate(sizeof(list), __FILE__, __LINE__, NULL);
  queue->next->thread = insert;
  queue->next->next = front;

  queue = queue->next;
  *q = queue;
  return;
}

//L: remove from queue
tcb* dequeue(list** q)
{
  list *queue = *q;
  if(queue == NULL)
  {
    return NULL;
  }
  //queue is the last element in a queue at level i
  //first get the thread control block to be returned
  list *front = queue->next;
  tcb *tgt = queue->next->thread;
  //check if there is only one element left in the queue
  //and assign null/free appropriately
  if(queue->next == queue)
  { 
    queue = NULL;
  }
  else
  {
    queue->next = front->next;
  }
  mydeallocate(front, __FILE__, __LINE__, NULL);

  
  if(tgt == NULL)
  {printf("WE HAVE A PROBLEM IN DEQUEUE\n");}

  *q = queue;
  return tgt;
}

//L: insert to list
void l_insert(list** q, tcb* jThread) //Non-circular Linked List
{
  list *queue = *q;

  if(queue == NULL)
  {
    queue = (list*)myallocate(sizeof(list),__FILE__, __LINE__, NULL);
    queue->thread = jThread;
    queue->next = NULL;
    *q = queue;
    return;
  }

  list *newNode = (list*)myallocate(sizeof(list), __FILE__, __LINE__, NULL);
  newNode->thread = jThread;

  //L: append to front of LL
  newNode->next = queue;
  
  queue = newNode;
  *q = queue;
  return;
}

//L: remove from list
tcb* l_remove(list** q)
{
  list *queue = *q;

  if(queue == NULL)
  {
    return NULL;
  }

  list *temp = queue;
  tcb *ret = queue->thread;
  queue = queue->next;
  mydeallocate(temp, __FILE__, __LINE__, NULL);
  *q = queue;
  return ret;
}


//L: Search table for a tcb given a uintint page_request = ceil((double)(size_req_int)/ PAGE_SIZE);
tcb* thread_search(my_pthread_t tid)
{
  int key = tid % MAX_SIZE;
  tcb *ret = NULL;

  list *temp = allThreads[key];
  while(allThreads[key] != NULL)
  {
    if(allThreads[key]->thread->tid == tid)
    {
      ret = allThreads[key]->thread;
      break;
    }
    allThreads[key] = allThreads[key]->next;
  }

  allThreads[key] = temp;

  return ret;
}

void initializeMainContext()
{
  tcb *mainThread = (tcb*)myallocate(sizeof(tcb), __FILE__, __LINE__, NULL);
  ucontext_t *mText = (ucontext_t*)myallocate(sizeof(ucontext_t), __FILE__, __LINE__, NULL);
  getcontext(mText);
  mText->uc_link = &cleanup;

  mainThread->context = mText;
  mainThread->tid = 0;
  mainThread->priority = 0;
  mainThread->joinQueue = NULL;
  mainThread->jVal = NULL;
  mainThread->retVal = NULL;
  mainThread->status = READY;

  mainRetrieved = 1;

  l_insert(&allThreads[0], mainThread);

  currentThread = mainThread;
}

void initializeGarbageContext()
{
  memset(&sig,0,sizeof(mySig));
  sig.sa_handler = &scheduler;
  sigaction(SIGVTALRM, &sig,NULL);
  initializeQueues(runningQueue); //set everything to NULL
    
  //Initialize garbage collector
  getcontext(&cleanup);
  cleanup.uc_link = NULL;
  cleanup.uc_stack.ss_sp = myallocate(STACK_S, __FILE__, __LINE__, NULL);
  cleanup.uc_stack.ss_size = STACK_S;
  cleanup.uc_stack.ss_flags = 0;
  makecontext(&cleanup, (void*)&garbage_collection, 0);

  //L: set thread count
  threadCount = 1;
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{

  if(!mainRetrieved)
  {
    initializeGarbageContext();
  }

  notFinished = 1;

  //L: Create a thread context to add to scheduler
  ucontext_t* task = (ucontext_t*)myallocate(250sizeof(ucontext_t), __FILE__, __LINE__, NULL);
  getcontext(task);
  task->uc_link = &cleanup;
  task->uc_stack.ss_sp = myallocate(STACK_S, __FILE__, __LINE__, NULL);
  task->uc_stack.ss_size = STACK_S;
  task->uc_stack.ss_flags = 0;
  makecontext(task, (void*)function, 1, arg);

  tcb *newThread = (tcb*)myallocate(sizeof(tcb), __FILE__, __LINE__, NULL);
  newThread->context = task;
  newThread->tid = threadCount;
  newThread->priority = 0;
  newThread->joinQueue = NULL;
  newThread->jVal = NULL;
  newThread->retVal = NULL;
  newThread->status = READY;

  *thread = threadCount;
  threadCount++;

  enqueue(&runningQueue[0], newThread);
  int key = newThread->tid % MAX_SIZE;
  l_insert(&allThreads[key], newThread);

  notFinished = 0;

  //L: store main context

  if (!mainRetrieved)
  {
    initializeMainContext();

    raise(SIGVTALRM);
  }
  //printf("New thread created: TID %d\n", newThread->tid);
  
  
  return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield()
{
  if(!mainRetrieved)
  {
    initializeGarbageContext();
    initializeMainContext();
  }
  //L: return to signal handler/scheduler
  currentThread->status = YIELD;
  return raise(SIGVTALRM);
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr)
{
  if(!mainRetrieved)
  {
    initializeGarbageContext();
    initializeMainContext();
  }
  //L: call garbage collection
  currentThread->jVal = value_ptr;
  setcontext(&cleanup);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr)
{
  if(!mainRetrieved)
  {
    initializeGarbageContext();
    initializeMainContext();
  }
  notFinished = 1;
  //L: make sure thread can't wait on self
  if(thread == currentThread->tid)
  {return -1;}

  tcb *tgt = thread_search(thread);
  
  if(tgt == NULL)
  {
    return -1;
  }
  
  //Priority Inversion Case
  tgt->priority = 0;

  l_insert(&tgt->joinQueue, currentThread);

  currentThread->status = JOIN;

  notFinished = 0;
  raise(SIGVTALRM);

  if(value_ptr == NULL)
  {return 0;}

  *value_ptr = currentThread->retVal;

  return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  notFinished = 1;
  my_pthread_mutex_t m = *mutex;
  
  m.available = 1;
  m.locked = 0;
  m.holder = -1; //holder represents the tid of the thread that is currently holding the mutex
  m.queue = NULL;

  *mutex = m;
  notFinished = 0;
  return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex)
{
  if(!mainRetrieved)
  {
    initializeGarbageContext();
    initializeMainContext();
  }
  notFinished = 1;

  //FOR NOW ASSUME MUTEX WAS INITIALIZED
  if(!mutex->available)
  {return -1;}

  while(__atomic_test_and_set((volatile void *)&mutex->locked,__ATOMIC_RELAXED))
  {
    //the reason why we reset notFinished to one here is that when coming back
    //from a swapcontext, notFinished may be zero and we can't let the operations
    //in the loop be interrupted
    notFinished = 1;
    enqueue(&mutex->queue, currentThread);
    currentThread->status = MUTEX_WAIT;
    //we need to set notFinished to zero before going to scheduler
    notFinished = 0;
    raise(SIGVTALRM);
  }

  if(!mutex->available)
  {
    mutex->locked = 0;
    return -1;
  }

  //Priority Inversion Case
  currentThread->priority = 0;
  mutex->holder = currentThread->tid;
  
  notFinished = 0;
  return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
  if(!mainRetrieved)
  {
    initializeGarbageContext();
    initializeMainContext();
  }
  //NOTE: handle errors: unlocking an open mutex, unlocking mutex not owned by calling thread, or accessing unitialized mutex

  notFinished = 1;

  //ASSUMING mutex->available will be initialized to 0 by default without calling init
  //available in this case means that mutex has been initialized or destroyed (state variable)
  if(!mutex->available || !mutex->locked || mutex->holder != currentThread->tid)
  {return -1;}

  mutex->locked = 0;
  mutex->holder = -1;

  tcb* muThread = dequeue(&mutex->queue);
  
  if(muThread != NULL)
  {
    //Priority Inversion Case
    muThread->priority = 0;
    enqueue(&runningQueue[0], muThread);
  }

  notFinished = 0;

  return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex)
{
  notFinished = 1;
  my_pthread_mutex_t m = *mutex;
  //L: prevent threads from accessing while mutex is in destruction process
  m.available = 0;
  notFinished = 0;

  //L: if mutex still locked, wait for thread to release lock
  while(m.locked)
  {raise(SIGVTALRM);}

  tcb *muThread;
  while(m.queue != NULL)
  {
    muThread = dequeue(&m.queue);
    enqueue(&runningQueue[muThread->priority], muThread);
  }

  *mutex = m;
  return 0;
};

void initializeQueues(list** runQ) 
{
  int i;
  for(i = 0; i < MAX_SIZE; i++) 
  {
    runningQueue[i] = NULL;
    allThreads[i] = NULL;
  }
  
}

/*				MEMORY MANAGEMENT
==================================================================================================================
==================================================================================================================
==================================================================================================================
==================================================================================================================
==================================================================================================================
*/


static void memory_manager(int signum, siginfo_t *si, void *ignoreMe)
{

  char *src_page = (char *)si->si_addr;
  int src_offset = src_page - &PHYS_MEM[MEM_SIZE/2];
  int src_pageNum = src_offset/PAGE_SIZE;
  /*if(firstPage == -1)
  {
    uintptr_t startFrame = (uintptr_t)&PHYS_MEMORY[MEM_SIZE/2];
    firstPage = (startFrame >> 12) & 0xfffff;
  }*/

  //pageNum -= firstPage; //how many pages away from the first page in user memory

  if(pageFlag == 0) //thread/user or library/OS accessing invalid memory
  {
    printf("->Segmentation Fault");
    exit(EXIT_FAILURE);
  }

  if(pageFlag == CREATE_PAGE) //first time we call malloc for any thread
  {
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_READ | PROT_WRITE);
    //check if page we are on is not owned by any thread at all (may need to swap out)
    //requires creating new pages
    char freeBit = frameMetaPhys[src_pageNum].isFree;
    int tid = frameMetaPhys[src_pageNum].owner;
    //find number of pages needed for allocation
    int num_of_pages_req = ceil((double)(request_size+(sizeof(char)*3))/ PAGE_SIZE);
    //look to see if there's enough room on physical and disk to fit new pages requested
    int i;
    //insert frame metas either in physical or disk for the malloc request
    //check physical memory first
    //assuming for the first malloc in a thread
    for(i = 0; i < META_PHYS_S && num_of_pages_req > 0; i++)
    {
      if(frameMetaPhys[i].isFree)
      {
        //if this page isn't owned by any thread
        frameMetaPhys[i].isFree = 0; 
        frameMetaPhys[i].owner = currentThread->tid;
        frameMetaPhys[i].pageNum = i;
        swapMe(i, 0); //swap the meta we just inserted into the right spot
        num_of_pages_req--;
      }
      else if(frameMetaPhys[i].owner != currentThread->tid)
      {
        //if this page is owned by another thread, need to swap it out after inserting page to somewhere in physical mem or swap file
        //first find open page in physical or swap file
        int j;
        int foundInPhys = 0;
        int foundInDisk = 0;
        //check phys
        for(j = i + 1; j < META_PHYS_S; j++)
        {
          if(frameMetaPhys[j].isFree)
          {
            frameMetaPhys[j].isFree = 0;
            frameMetaPhys[j].owner = currentThread->tid;
            frameMetaPhys[j].pageNum = i;
            swapMe(i, 0); //swap the meta we just inserted into the right spot
            num_of_pages_req--;
            foundInPhys = 1;
            break;
          }
        }
        //check disk
        if(!foundInPhys)
        {
          for(j = 0; j < META_SWAP_S; j++)
          {
            if(frameMetaSwap[j].isFree)
            {
              frameMetaSwap[j].isFree = 0;
              frameMetaSwap[j].owner = currentThread->tid;
              frameMetaSwap[j].pageNum = i;
              swapMe(i, 1); //swap the meta we just inserted into the right spot
              num_of_pages_req--;
              foundInDisk = 1;
              break;
            }
          }
          if(!foundInDisk)
          {
            printf("Ran out of memory in both physical and disk\n");
            exit(-1);
          }
        }
      }
    }
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_NONE);
    for(i = 0; i < META_PHYS_S; i++)
    {
      //unprotect pages belonging to the current thread we are context siwtching to
      if(frameMetaPhys[i].owner == currentThread->tid)
      {
        mprotect(&PHYS_MEMORY[MEM_SIZE/2 + (i * PAGE_SIZE)], PAGE_SIZE, PROT_READ | PROT_WRITE);
      }
    }
  }

  if(pageFlag == EXTEND_PAGES) //free block within a page owned by a thread in physical mem found
  {
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_READ | PROT_WRITE);
    //need to create frame meta entries to fit size of request
    //TODO: find offset from current page

    src_page = startAddr;
    src_offset = src_page - &PHYS_MEM[MEM_SIZE/2];
    src_pageNum = src_offset/PAGE_SIZE;
    int headerSize = sizeof(char) * 3;
    //check if header is split among two pages
    int next_pageNum = src_pageNum + 1;
    int num_of_pages_to_free = 0;
    int difference = 0;
    if(next_pageNum < META_PHYS_S && src_page + headerSize + blockToFreeSize <= &PHYS_MEM[next_pageNum * PAGE_SIZE])
    {
      //dont need to free any frames because the block to free is contained within one page
      printf("ayyyyyyyyyyyyyyyyyyyy\n");
      
    }
    else{
      if(next_pageNum < META_PHYS_S && (startAddr + (sizeof(char) * 3)) > &PHYS_MEM[next_pageNum * PAGE_SIZE])
      {
        next_pageNum += 1;
        //header is split between two frames
        if(next_pageNum < META_PHYS_S && src_page + headerSize + blockToFreeSize <= &PHYS_MEM[next_pageNum * PAGE_SIZE])
        {
          //dont need to free any frames because the block to free is contained within one page
          printf("ayyyyyyyyyyyyyyyyyyyy\n");
      
        }
        else
        {
          //block not contained within one frame
          //find the difference between the address of the beginning of the next page and the address of the next header block
          difference = (src_page + (sizeof(char) * 3) + blockToFreeSize) - &PHYS_MEM[next_pageNum * PAGE_SIZE]
        }
      }
      else if(next_pageNum < META_PHYS_S)
      {
        //header is not split between two frames
        //find the difference between the address of the beginning of the next page and the address of the next header block
        difference = (src_page + (sizeof(char) * 3) + blockToFreeSize) - &PHYS_MEM[next_pageNum * PAGE_SIZE]     
      }
      num_of_pages_to_malloc = ceil((double)(difference)/ PAGE_SIZE);
    }
    //look to see if there's enough room on physical and disk to fit new pages needed for request
    int i;
    //insert frame metas either in physical or disk for the malloc request
    //check physical memory first
    //assuming for the first malloc in a thread
    for(i = next_pageNum; i < META_PHYS_S && num_of_pages_req > 0; i++)
    {
      if(frameMetaPhys[i].isFree)
      {
        //if this page isn't owned by any thread
        frameMetaPhys[i].isFree = 0; 
        frameMetaPhys[i].owner = currentThread->tid;
        frameMetaPhys[i].pageNum = i;
        swapMe(i, 0); //swap the meta we just inserted into the right spot
        num_of_pages_req--;
      }
      else if(frameMetaPhys[i].owner != currentThread->tid)
      {
        //if this page is owned by another thread, need to swap it out after inserting page to somewhere in physical mem or swap file
        //first find open page in physical or swap file
        int j;
        int foundInPhys = 0;
        int foundInDisk = 0;
        //check phys
        for(j = i + 1; j < META_PHYS_S; j++)
        {
          if(frameMetaPhys[j].isFree)
          {
            frameMetaPhys[j].isFree = 0;
            frameMetaPhys[j].owner = currentThread->tid;
            frameMetaPhys[j].pageNum = i;
            swapMe(i, 0); //swap the meta we just inserted into the right spot
            num_of_pages_req--;
            foundInPhys = 1;
            break;
          }
        }
        //check disk
        if(!foundInPhys)
        {
          for(j = 0; j < META_SWAP_S; j++)
          {
            if(frameMetaSwap[j].isFree)
            {
              frameMetaSwap[j].isFree = 0;
              frameMetaSwap[j].owner = currentThread->tid;
              frameMetaSwap[j].pageNum = i;
              swapMe(i, 1); //swap the meta we just inserted into the right spot
              num_of_pages_req--;
              foundInDisk = 1;
              break;
            }
          }
          if(!foundInDisk)
          {
            printf("Ran out of memory in both physical and disk\n");
            exit(-1);
          }
        }
      }
    }
    
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_NONE);
    for(i = 0; i < META_PHYS_S; i++)
    {
      //unprotect pages belonging to the current thread we are context siwtching to
      if(frameMetaPhys[i].owner == currentThread->tid)
      {
        mprotect(&PHYS_MEMORY[MEM_SIZE/2 + (i * PAGE_SIZE)], PAGE_SIZE, PROT_READ | PROT_WRITE);
      }
    }
  }
  if(pageFlag == CONTEXT_SWITCH)
  {
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_READ | PROT_WRITE);
    //find all pages that belong to current thread in physical memory
    int i;
    for(i = 0; i < META_PHYS_S; i++)
    {
      if(frameMetaPhys[i].owner == currentThread->tid)
      {
        swapMe(i, 0);
      }
    }
    //find all pages that belong to current thread in disk
    for(i = 0; i < META_SWAP_S; i++)
    {
      if(frameMetaSwap[i].owner == currentThread->tid)
      {
        swapMe(i, 1);
      }
    }
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_NONE);
    for(i = 0; i < META_PHYS_S; i++)
    {
      //unprotect pages belonging to the current thread we are context siwtching to
      if(frameMetaPhys[i].owner == currentThread->tid)
      {
        mprotect(&PHYS_MEMORY[MEM_SIZE/2 + (i * PAGE_SIZE)], PAGE_SIZE, PROT_READ | PROT_WRITE);
      }
    }
  }
  
  if(pageFlag == FREE_FRAMES)
  {
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_READ | PROT_WRITE);

    src_page = startAddr;
    src_offset = src_page - &PHYS_MEM[MEM_SIZE/2];
    src_pageNum = src_offset/PAGE_SIZE;
    int headerSize = sizeof(char) * 3;
    //check if header is split among two pages
    int next_pageNum = src_pageNum + 1;
    int num_of_pages_to_free = 0;
    int difference = 0;
    if(next_pageNum < META_PHYS_S && src_page + headerSize + blockToFreeSize <= &PHYS_MEM[next_pageNum * PAGE_SIZE])
    {
      //dont need to free any frames because the block to free is contained within one page
      printf("ayyyyyyyyyyyyyyyyyyyy\n");
      
    }
    else{
      if(next_pageNum < META_PHYS_S && (startAddr + (sizeof(char) * 3)) > &PHYS_MEM[next_pageNum * PAGE_SIZE])
      {
        next_pageNum += 1;
        //header is split between two frames
        if(next_pageNum < META_PHYS_S && src_page + headerSize + blockToFreeSize <= &PHYS_MEM[next_pageNum * PAGE_SIZE])
        {
          //dont need to free any frames because the block to free is contained within one page
          printf("ayyyyyyyyyyyyyyyyyyyy\n");
      
        }
        else
        {
          //block not contained within one frame
          //find the difference between the address of the beginning of the next page and the address of the next header block
          difference = (src_page + (sizeof(char) * 3) + blockToFreeSize) - &PHYS_MEM[next_pageNum * PAGE_SIZE]
        }
      }
      else if(next_pageNum < META_PHYS_S)
      {
        //header is not split between two frames
        //find the difference between the address of the beginning of the next page and the address of the next header block
        difference = (src_page + (sizeof(char) * 3) + blockToFreeSize) - &PHYS_MEM[next_pageNum * PAGE_SIZE]     
      }
      num_of_pages_to_free = floor((double)(difference)/ PAGE_SIZE);
      //free pages' frame meta
      int i;
      for(i = next_pageNum; i < next_pageNum + 1 + num_of_pages_to_free; i++)
      {
        frameMetaPhys[i].isFree = 1;
        frameMetaPhys[i].owner = 0;
        frameMetaPhys[i].pageNum = 0;
      }
    }
    
    mprotect(PHYS_MEMORY, MEM_SIZE, PROT_NONE);
    for(i = 0; i < META_PHYS_S; i++)
    {
      //unprotect pages belonging to the current thread we are context siwtching to
      if(frameMetaPhys[i].owner == currentThread->tid)
      {
        mprotect(&PHYS_MEMORY[MEM_SIZE/2 + (i * PAGE_SIZE)], PAGE_SIZE, PROT_READ | PROT_WRITE);
      }
    }
  }



}

void setMem()
{
  //initialize physical memory
  PHYS_MEMORY = (unsigned char*)memalign(PAGE_SIZE, MEM_SIZE);
  bzero(PHYS_MEMORY, MEM_SIZE);

  firstPage=-1;

  //set up two blocks of size 4 MB, first half is user memory, second half is OS/library memory
  //create blocks of size MEM_SIZE/2
  unsigned char freeBit = 0x80;
  int totalSize = (MEM_SIZE/2) - 3; //-3 because headers are 3 bytes
  int pIndex = MEM_SIZE/2;

  //Allocation metadata: leftmost bit for free bit, 23 bits for allocation size
  //first the OS memory
  PHYS_MEMORY[0] = freeBit | ((totalSize >> 16) & 0x7f);
  PHYS_MEMORY[1] = (totalSize >> 8) & 0xff;
  PHYS_MEMORY[2] = totalSize & 0xff;

  //second the user memory
  PHYS_MEMORY[pIndex] = freeBit | ((totalSize >> 16) & 0x7f);
  PHYS_MEMORY[pIndex+1] = (totalSize >> 8) & 0xff;
  PHYS_MEMORY[pIndex+2] = totalSize & 0xff;    

  //initialize swap file
  initializeDisk();
    
  //protect entire memory
  mprotect(PHYS_MEMORY, MEM_SIZE, PROT_NONE);


  //set up signal handler
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



/*Creates the 16 MB Swap File*/
void initializeDisk()
{
  char *swapper = "swapFile";
  swapFileFD = open(swapper,O_CREAT | O_RDWR | O_TRUNIC, S_IRUSR | S_IWUSR);
  lseek(swapFileFD,0,SEEK_SET);
  static unsigned char swapFileBuffer[PAGE_SIZE * PAGE_SIZE];
  bzero(swapFileBuffer,PAGE_SIZE*PAGE_SIZE);
  write(swapFileFD,swapFileBuffer,PAGE_SIZE*PAGE_SIZE);
  lseek(swapFileFD,0,SEEK_SET);
}


/*Swaps frames out for one another, exact frames depend upon the context*/
void swapMe(int pageLocation, int startOnDisk)
{
  //Searching for frame in PHYS_MEMORY
  int i = 0;
  if(!startOnDisk){
    for(i=0; i < META_PHYS_S; i++)
    {
      if(frameMetaPhys[i].owner == currentThread->tid && frameMetaPhys[i].pageNum == pageLocation) //find page of current thread in physical memory
      {
        //unprotect memory of physical memory at PAGE_SIZE * pageLocation
        mprotect(PHYS_MEMORY[pageLocation*PAGE_SIZE], PAGE_SIZE, PROT_READ | PROT_WRITE);
        char buffer1[PAGE_SIZE];
        char buffer2[PAGE_SIZE];
        //swap out contents of physical memory
        memcpy(buffer1,PHYS_MEMORY[pageLocation*PAGE_SIZE],PAGE_SIZE); //what's being swapped out
        memcpy(buffer2,PHYS_MEMORY[i*PAGE_SIZE],PAGE_SIZE); //what's being swapped in
        memcpy(PHYS_MEMORY[pageLocation*PAGE_SIZE],buffer2,PAGE_SIZE);
        memcpy(PHYS_MEMORY[i*PAGE_SIZE],buffer1,PAGE_SIZE);
        //swap out contents of meta data?
        frameMeta fm1;
        frameMeta fm2;
        memcpy(&fm1, frameMetaPhys[pageLocation], sizeof(fm1));
        memcpy(&fm2, frameMetaPhys[i], sizeof(fm2));
        memcpy(frameMetaPhys[pageLocation], &fm2, sizeof(fm1));
        memcpy(frameMetaPhys[i], &fm1, sizeof(fm2));
        return;
      }
    }
  }

  //Not in physcial, need to search Swap File
  lseek(swapFileFD,0,SEEK_SET);
  for(i=0; i < META_SWAP_S; i++)
  {
    if(frameMetaSwap[i].owner == currentThread->tid && frameMetaSwap[i].pageNum == pageLocation) //find page of current thread in swapfile
    {
      //unprotect memory of physical memory at PAGE_SIZE * pageLocation
      mprotect(PHYS_MEMORY[pageLocation*PAGE_SIZE], PAGE_SIZE, PROT_READ | PROT_WRITE);
      //swap out contents of physical memory with page found in disk
      char buffer1[PAGE_SIZE];
      char buffer2[PAGE_SIZE];
      memcpy(buffer1,PHYS_MEMORY[pageLocation*PAGE_SIZE],PAGE_SIZE); //what's being swapped out
      lseek(swapFileFD, i * PAGE_SIZE, SEEK_SET); //what's being swapped in
      read(swapFileFD, buffer2, PAGE_SIZE);
      memcpy(PHYS_MEMORY[pageLocation*PAGE_SIZE],buffer2,PAGE_SIZE);
      write(swapFileFD, buffer1, PAGE_SIZE);
      //swap out contents of meta data?
      frameMeta fm1;
      frameMeta fm2;
      memcpy(&fm1, frameMetaPhys[pageLocation], sizeof(fm1));
      memcpy(&fm2, frameMetaSwap[i], sizeof(fm2));
      memcpy(frameMetaPhys[pageLocation], &fm2, sizeof(fm1));
      memcpy(frameMetaSwap[i], &fm1, sizeof(fm2));
      return;
    }
  }
}

void* myallocate(size_t size_req, char *fileName, int line, char src)
{
 
  //TODO: will we allow allocation greater than 4mb? Fuck no boy
  //initialize memory and disk
  if(!isSet)
  {
    setMem();
  }

  if(size_req <= 0 || size_req > (MEM_SIZE/2) - 3)//don't allow allocations of size 0; would cause internal fragmentation due to headers
  {
    return NULL;
  }

  request_size = size_req;//set request size global to transfer to signal handler

  unsigned int start_index, bound;
  unsigned int meta; 
  unsigned int isFree, blockSize;  

  //pageFlag = 1;//tell signal handler an occurrence of segfault would have come from malloc

  //if library call
  if(src == 0)
  {
    mprotect(PHYS_MEMORY, MEM_SIZE/2, PROT_WRITE | PROT_READ);
    start_index = 0;
    bound = MEM_SIZE/2;
  }

  //if thread call
  else if(src == 1)
  {
    start_index = MEM_SIZE/2;
    bound = MEM_SIZE;
  }

  //reeaaally shouldn't ever happen but just in case
  else
  {
    printf("Error on source of call to malloc. Exiting...\n");
  }

  while(start_index < bound)
  {
    //extract free bit & block size from header
    pageFlag = CREATE_PAGE;
    meta = (PHYS_MEMORY[start_index] << 16) | (PHYS_MEMORY[start_index+1] << 8) | (PHYS_MEMORY[start_index+2]); //if we encounter this spot in memory where there's no page, we'll go to handler to create that new page
    pageFlag = CLEAR_FLAG;
    isFree = (meta >> 23) & 0x1;
    blockSize = meta & 0x7fffff;
    //valid block found
    if(isFree && blockSize >= size_req)
    {
      
      int prev_index = start_index; //in the case that we dont split, we need to increase the blockSize
      //fill in metadata for allocated block

      start_index += (size_req + 3);//jump to next block to place new header
      
      // fill in metadata for new free block
      int sizeLeft = blockSize - size_req - 3;

      if(sizeLeft > 0)//only add new header if enough space remains for allocation of at least one byte
      {
        pageFlag = EXTEND_PAGES;
        startAddr = &PHYS_MEMORY[start_index];//pass address of block to global variable in event signal handler is raised (also for return value)
        raise(SIGSEGV);
        pageFlag = CLEAR_FLAG;
        PHYS_MEMORY[prev_index] = (size_req >> 16) & 0x7f;
        PHYS_MEMORY[prev_index+1] = (size_req >> 8) & 0xff;
        PHYS_MEMORY[prev_index+2] = size_req & 0xff;
        PHYS_MEMORY[start_index] = 0x80 | ((sizeLeft >> 16) & 0x7f);
        PHYS_MEMORY[start_index+1] = (sizeLeft >> 8) & 0xff;
        PHYS_MEMORY[start_index+2] = sizeLeft & 0xff;
      }
      else
      {
        pageFlag = EXTEND_PAGES;
        startAddr = &PHYS_MEMORY[start_index];//pass address of block to global variable in event signal handler is raised (also for return value)
        request_size = blockSize;
        raise(SIGSEGV);
        pageFlag = CLEAR_FLAG;
        //in the event of no split, we need to give extra memory
        PHYS_MEMORY[prev_index] = (blockSize >> 16) & 0x7f;
        PHYS_MEMORY[prev_index+1] = (blockSize >> 8) & 0xff;
        PHYS_MEMORY[prev_index+2] = blockSize & 0xff;
      }
      pageFlag = CLEAR_FLAG;//reset pageFlag
      mprotect(PHYS_MEMORY, MEM_SIZE/2, PROT_NONE);
      return startAddr;

    }

    else
    {
      //go to next subsequent header
      start_index += (blockSize + 3);
    }


  }
  
  //If reached, no valid block could be allocated
  pageFlag = CLEAR_FLAG;//reset pageFlag
  mprotect(PHYS_MEMORY, MEM_SIZE/2, PROT_NONE);
  return NULL;
}



void mydeallocate(void *ptr, char *fileName, int line, char src)
{
  if(ptr == NULL)//don't do anything if null pointer passed
  {
    return;
  }
  if(ptr < PHYS_MEMORY || ptr > &PHYS_MEMORY[MEM_SIZE - 1])
  {
    //address the user entered is not within physical memory
    raise(SIGSEGV);
  }
  int start_index;
  int bound;
  int prevBlock = -1;
  unsigned int meta, nextMeta, prevMeta;
  unsigned char isFree, nextFree, prevFree;
  unsigned int blockSize, nextSize, prevSize;
  char *leftoverBlock;
  if(src == 0)//library
  {
    mprotect(PHYS_MEMORY, MEM_SIZE/2, PROT_WRITE | PROT_READ);
    start_index = 0;
    bound = MEM_SIZE/2;

    while(start_index < bound)
    {
      if(&PHYS_MEMORY[start_index] == ptr)
      {
        meta = (PHYS_MEMORY[start_index] << 16) | (PHYS_MEMORY[start_index+1] << 8) | (PHYS_MEMORY[start_index+2]);
        isFree = (meta >> 23) & 0x1;
        blockSize = meta & 0x7fffff;

        if(isFree)//block has already been freed
        {
	  printf("Attempted Double Free\n");
	  exit(EXIT_FAILURE);
        }

        PHYS_MEMORY[start_index] = 0x80;//reset free bit

        //coalesce with next block
        if(PHYS_MEMORY[start_index] + blockSize + 3 < MEM_SIZE)
        {
  	  nextMeta = (PHYS_MEMORY[start_index + blockSize + 3] << 16) | (PHYS_MEMORY[start_index + blockSize + 4] << 8) | (PHYS_MEMORY[start_index + blockSize + 5]);
          nextFree = (nextMeta >> 23) & 0x1;
          nextSize = nextMeta & 0x7fffff;

	  if(nextFree)
	  {
	    blockSize += nextSize + (sizeof(char) * 3);
	    PHYS_MEMORY[start_index] = 0x80 | ((blockSize_ >> 16) & 0x7f);
            PHYS_MEMORY[start_index+1] = (blockSize >> 8) & 0xff;
            PHYS_MEMORY[start_index+2] = blockSize & 0xff;
	  }

          //coalesce with prev block
          if(prevBlock != -1)
          {
	    prevMeta = (PHYS_MEMORY[prevBlock] << 16) | (PHYS_MEMORY[prevBlock+1] << 8) | (PHYS_MEMORY[prevBlock+2]);
            prevFree = (prevMeta >> 23) & 0x1;
            prevSize = prevMeta & 0x7fffff;

	    if(prevFree)
	    {
	      blockSize += prevSize + (sizeof(char)*3);
              PHYS_MEMORY[prevBlock] = 0x80 | ((blockSize >> 16) & 0x7f);
              PHYS_MEMORY[prevBlock+1] = (blockSize >> 8) & 0xff;
              PHYS_MEMORY[prevBlock+2] = blockSize & 0xff;
	    }

          }
        }
        break;
      }
      else
      {
        prevBlock = start_index;
        start_index += blockSize + (sizeof(char) * 3);
      }

    }
    mprotect(PHYS_MEMORY, MEM_SIZE/2, PROT_NONE);
  }

  else //user thread
  {
    start_index = MEM_SIZE/2;
    bound = MEM_SIZE;

    while(start_index < bound)
    {
      if(&PHYS_MEMORY[start_index] == ptr)
      {
        meta = (PHYS_MEMORY[start_index] << 16) | (PHYS_MEMORY[start_index+1] << 8) | (PHYS_MEMORY[start_index+2]);
        isFree = (meta >> 23) & 0x1;
        blockSize = meta & 0x7fffff;
        leftoverBlock = &PHYS_MEMORY[start_index];
        if(isFree)//block has already been freed
        {
	  printf("Attempted Double Free\n");
	  exit(EXIT_FAILURE);
        }

        PHYS_MEMORY[start_index] = 0x80;//reset free bit

        //coalesce with next block
        if(PHYS_MEMORY[start_index] + blockSize + 3 < MEM_SIZE)
        {
  	  nextMeta = (PHYS_MEMORY[start_index + blockSize + 3] << 16) | (PHYS_MEMORY[start_index + blockSize + 4] << 8) | (PHYS_MEMORY[start_index + blockSize + 5]);
          nextFree = (nextMeta >> 23) & 0x1;
          nextSize = nextMeta & 0x7fffff;

	  if(nextFree)
	  {
	    blockSize += nextSize + (sizeof(char) * 3);
	    PHYS_MEMORY[start_index] = 0x80 | ((blockSize_ >> 16) & 0x7f);
            PHYS_MEMORY[start_index+1] = (blockSize >> 8) & 0xff;
            PHYS_MEMORY[start_index+2] = blockSize & 0xff;
	  }

          //coalesce with prev block
          if(prevBlock != -1)
          {
	    prevMeta = (PHYS_MEMORY[prevBlock] << 16) | (PHYS_MEMORY[prevBlock+1] << 8) | (PHYS_MEMORY[prevBlock+2]);
            prevFree = (prevMeta >> 23) & 0x1;
            prevSize = prevMeta & 0x7fffff;

	    if(prevFree)
	    {
	      blockSize += prevSize + (sizeof(char)*3);
              PHYS_MEMORY[prevBlock] = 0x80 | ((blockSize >> 16) & 0x7f);
              PHYS_MEMORY[prevBlock+1] = (blockSize >> 8) & 0xff;
              PHYS_MEMORY[prevBlock+2] = blockSize & 0xff;
              leftoverBlock = &PHYS_MEMORY[prevBlock];
	    }

          }
        }
        startAddr = leftoverBlock;
        blockToFreeSize = blockSize;
        pageFlag = FREE_FRAMES;
        raise(SIGSEGV);
        pageFlag = CLEAR_FLAG;
        break;
      }
      else
      {
        prevBlock = start_index;
        start_index += blockSize + (sizeof(char) * 3);
      }

    }    

  }
  
}

















