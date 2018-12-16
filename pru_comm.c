/* pru_comm.c - the source code for the pru side communications
 */
#include <stdint.h>
#include <string.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include "resource_table.h"
#include "pru_comm.h"

/********** defines ****************************/
#define VIRTIO_CONFIG_S_DRIVER_OK	4

/********* global variable definitions *********/
register volatile unsigned __R31;
struct pru_rpmsg_transport transport;
unsigned short rpmsg_src,rpmsg_dst;
unsigned char buffer[RPMSG_BUF_SIZE], pbuffer[RPMSG_BUF_SIZE];
unsigned short buffer_state=0,plen=0;
unsigned char Ndata;
unsigned short (*c[Maxc])(void); //command functions
unsigned char cd[Maxc];//command codes
unsigned char (*a[Maxc])(void); //action functions
unsigned char ad[Maxc];//action codes
int Ncc=0;
int Naa=0;
unsigned char command_str[RPMSG_BUF_SIZE],*pcommand_str;
unsigned char Ncommand;

/******** global external variable definitions */
unsigned char Nvalue;
unsigned char rpmsg_errno;


void reset_command(void){
  Ncommand = 2;
  pcommand_str = &command_str[2];
}

void init_command(void){
  command_str[0] = 'a';
  command_str[1] = 'f';
  reset_command();
}

//look at this again and see if you need to return to stepOne if you don't make progress on the other steps
//its possible that this can hang and never move on
unsigned short rpmsg_init(void){
  volatile unsigned char *status;
  static unsigned short rpmsg_state=stepOne;
  
  switch (rpmsg_state){
  case stepOne:
    CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
    rpmsg_state = stepTwo;
    break;
  case stepTwo:
    status = &resourceTable.rpmsg_vdev.status;
    if((*status & VIRTIO_CONFIG_S_DRIVER_OK)){       
      pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);
      rpmsg_state = stepThree;
    }
    break;
  case stepThree:
    if (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) == PRU_RPMSG_SUCCESS){
      rpmsg_state = Initialized;
      init_command();
    }
    break;
  }
  return(rpmsg_state);
}

void listen(void) {
  int retval;
  
  if(buffer_state == 1) return;
  if(! (__R31 & HOST_INT)) return; // return if ARM has not kicked us
  if ((retval=pru_rpmsg_receive(&transport, 
                                &rpmsg_src, 
                                &rpmsg_dst, 
                                buffer, 
                                &plen)) == PRU_RPMSG_SUCCESS){
    buffer_state = 1;
    rpmsg_errno  = 0;
    return;
  }
  
  if(retval == PRU_RPMSG_NO_BUF_AVAILABLE) rpmsg_errno = 1;
  if(retval == PRU_RPMSG_BUF_TOO_SMALL)    rpmsg_errno = 2;
  
  /* Clear the event status */
  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST; 
  return;
}

unsigned short send(char *buf,int N) {
  int retval;

  retval = pru_rpmsg_send(&transport,rpmsg_dst, rpmsg_src, buf,N);
  rpmsg_errno = 0;
  if(retval == PRU_RPMSG_NO_BUF_AVAILABLE) rpmsg_errno = 3;
  if(retval == PRU_RPMSG_BUF_TOO_SMALL)    rpmsg_errno = 4;  

  return(retval);
}
