#include <stdint.h>
#include <stdlib.h>
#include <pru_cfg.h>
#include "pru_comm.h"

volatile register unsigned __R31;
extern unsigned short buffer_state,plen;
extern unsigned char *pbuffer;

#define BUF_SIZE 480

void randomize(char* buf) {
  int i;
  for(i=0;i<BUF_SIZE;i++) 
  	buf[i] = rand() % 256;
}

unsigned short send(char *buf,int N); //defined in pru_comm.c

void main(void)
{
	/* Allow OCP master port access by the PRU so the PRU can read external memories */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	while (1) {	        
	  if(rpmsg_init() == Initialized){
	    
	    listen();
	    
	    if(buffer_state != 0) {			
          randomize( pbuffer );
          send((char *)pbuffer, BUF_SIZE);
          buffer_state = 0;
	    }
	  }
	}
}
