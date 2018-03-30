//#include "my_malloc.h"
//#include "my_pthread_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE (4096 * 4096)
#define PAGE_SIZE 4096

int main()
{
	char *swap="swapper";
	int fd = open(swap,O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
	lseek(fd,0,SEEK_SET);
	static unsigned char swapFile[SIZE];
	bzero(swapFile,SIZE);
	write(fd,swapFile,SIZE);

	unsigned char page[PAGE_SIZE];
	unsigned char page2[PAGE_SIZE];
	unsigned char page3[PAGE_SIZE];
	bzero(page,PAGE_SIZE);
	bzero(page2,PAGE_SIZE);
	bzero(page3,PAGE_SIZE);
	page[0] = 0xff;
	page[1] = 0xff;
	memcpy(page2,page,PAGE_SIZE);
	lseek(fd,PAGE_SIZE*2,SEEK_SET);
	write(fd,page2,PAGE_SIZE);
	lseek(fd,0,SEEK_SET);
	lseek(fd,PAGE_SIZE*2,SEEK_SET);
	read(fd,page3,PAGE_SIZE);
	int result = (page3[0] << 8) | page3[1];
	printf("Please work ~ Int: %d\t,Hex: 0x%04x\n",result,result);
	return 0;
}
