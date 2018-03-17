#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/mman.h>
#include <string.h>

#define MEM (1024 * 4)

/*
GOAL: Trigger Signal Handler on EVERY reference to memAccess, not just the first
*/


char *memory, *fake_mem;
char **page;

void mem_handler(int signum, siginfo_t *si, void *ignoreMe)
{
	printf("Source: %p\n", si->si_addr);
	char **p = si->si_addr;
	
	int offset = (si->si_addr - (void*)page);
	printf("Offset: %d\n", offset);

	mprotect(page, MEM, PROT_READ | PROT_WRITE);
	*p = (memory + offset);
}

int main(int argc, char *argv[])
{
	fake_mem = "Failure";
	memory = "Is Success";
	page = (char**)memalign(MEM, MEM);
	page[0] = fake_mem;

	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = mem_handler;

	if(sigaction(SIGSEGV, &sa, NULL) == -1)
	{
		printf("Fatal error setting up signal handler\n");
		exit(EXIT_FAILURE);    //explode!
    	}

	mprotect(page, MEM, PROT_NONE);
	char *memAccess = page[0];
	memAccess += 3;
	printf("Attempt 1: %s\n", memAccess);

	//TODO: How do we trigger segfault here?
	printf("Attempt 2: %s\n", memAccess);

	free(page);
	return 0;
}
