#include <stdio.h>
#include <unistd.h>
#include "arm_comm.h"

#define N		10

int CASSY;

extern unsigned short buffer_state; //expose buffer state
extern short plen;
extern unsigned char *pbuffer;

void printbuf(char* buf, int len) {
  int pos=0;
  for(pos=0;pos<len;pos++) {
    if(pos%16==0) printf("\n");
    if(pos%4==0)  printf(" ");
    printf("%02x",buf[pos]);
  }
  printf("\n");
}

int main(void)
{
  int i;

  while(1){
    if(rpmsg_init(&CASSY) == Initialized){
      break;
    }
  }	

  for (i = 0; i < N; i++) {
    write(CASSY,"gief",7);
    listen(CASSY);
    if(buffer_state != 0){
//      printf("length: %d\n",plen);
      printbuf(pbuffer,plen);
      buffer_state = 0;
    }
  }  
  rpmsg_deinit(CASSY);
  return 0;
}

