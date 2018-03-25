#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>

#define MEM_SIZE (1024 * 1024 * 8)
#define DISK_SIZE (1024 * 1024 * 16)
#define PAGE_SIZE (1024 * 4)
#define MEM_SECTION (MEM_SIZE/2)


int main(int argc, char *argv[])
{
	unsigned char* test = (unsigned char*)memalign(PAGE_SIZE, MEM_SIZE * 2);
        int i = 1;
        while(i)
        {
          mprotect(test, MEM_SECTION, PROT_NONE);
	  mprotect(test, MEM_SECTION, PROT_NONE);
	  mprotect(test, MEM_SECTION, PROT_READ | PROT_WRITE);
	  mprotect(test, MEM_SECTION, PROT_READ | PROT_WRITE);
          printf("%i\n", i);
          i++;
        }
	mprotect(test, MEM_SECTION, PROT_NONE);
	mprotect(test, MEM_SECTION, PROT_NONE);
	mprotect(test, MEM_SECTION, PROT_READ | PROT_WRITE);
	mprotect(test, MEM_SECTION, PROT_READ | PROT_WRITE);

	return 0;
}
