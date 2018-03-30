#include "my_pthread_t.h"
void *func(void *cheese);
int main()
{
  pthread_t t;
  char *str = malloc(4194301);
  strcpy(str,"abcdefghijklmnopqrstuvwxyzNowIKnowMyABCsNextTimeFinishOSForMe");
  printf("In main [Before]\n");
  pthread_create(&t, NULL, func, NULL);
  //pthread_yield();
  pthread_join(t,NULL);
  printf("In main [After]: %s\n",str);
  pthread_exit(NULL);
  return 0;
}

void *func(void *cheese)
{
  printf("In func [Before]\n");
  char *str = malloc(4194301);
  strcpy(str,"aaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbacbbbbbbbbbbbbbbbbbbbbbbbbccccccccccccccccccccccccccccccccccccccccccqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
  printf("In func [After]\n");
  pthread_exit(NULL);
  return NULL;
}
