#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "arm_comm.h"

//This is where you decide which PRU you will be using
//#define CASSY_DEV "/dev/rpmsg_pru30"
#define CASSY_DEV "/dev/rpmsg_pru31"

#define REMOTEPROC_DEV "/sys/class/remoteproc/remoteproc2/state"
#define REMOTEPROC_FIRM "/sys/class/remoteproc/remoteproc2/firmware"
//if you want to name your program something other than the default, this line allows that value to be written to /lib/firmware and to the firmware variable
//I'm pretty sure you can write this variable into the makefile and pass it to the compiler so that you don't have to maintain the name in two different places
#define FIRMWARE "am335x-pru1-fw" 
#define FIRMWARE_SIZE 14

void (*c[Maxc])(void); //command functions
unsigned char cd[Maxc];//command codes
unsigned char (*a[Maxc])(void); //action functions
unsigned char ad[Maxc];//action codes
int Ncc=0;
int Naa=0;



unsigned short buffer_state=0;
unsigned char buffer[RPMSG_BUF_SIZE], *pbuffer;
short plen; 
unsigned char Ndata,ValidData;
unsigned char command_str[RPMSG_BUF_SIZE],*pcommand_str;
unsigned char Ncommand;

int flags;
//NOTE: rpmsg puts '\n' at end of string data rather than '\0'.
//If you use string functions, you have to replace '\n' with '\0'.
//Therefore, use strncmp rather than strcmp
//str should be cleaned out between compares; read might not put anything into it.
//By defining str as local to rpmsg_init and reentering the function between each step,
//it's possible that the system cleans it out or it's possible that the system uses
//whatever was in the memory

void rpmsg_deinit(int CASSY){
  fcntl(CASSY,F_SETFL,flags | O_NONBLOCK);
  close(CASSY);
}

unsigned short rpmsg_init(int *CASSY){
  static unsigned short rpmsg_state=stepOne;
  int n;
  static int remoteproc_state= -1;
  char str[10];

  switch(rpmsg_state){
  case stepOne: //open device, read from device, check for running
    printf("step one\n");
    if(remoteproc_state < 0){
      remoteproc_state  = open(REMOTEPROC_DEV,O_RDONLY);
      printf("%s %d\n",REMOTEPROC_DEV,remoteproc_state);
      return(rpmsg_state);
    }
    n = read(remoteproc_state,str,20);
    printf("n: %d, s: %s\n", n, str);
    if(n > 0) {
      if(strncmp("offline",str,7) == 0){ //strings match
	      rpmsg_state = stepThree; //write firmware
	      close(remoteproc_state);
	      remoteproc_state = -1;
	      return(rpmsg_state);
      }
      if(strncmp("running",str,7) == 0){ //strings match
	      rpmsg_state = stepTwo; //stop
	      close(remoteproc_state);
	      remoteproc_state = -1;
	      return(rpmsg_state);
      }
    }
    break;
  case stepTwo: //stop
    printf("step two\n");
    if(remoteproc_state < 0){
      remoteproc_state  = open(REMOTEPROC_DEV,O_RDWR);
      return(rpmsg_state);
    }
    if((n=write(remoteproc_state,"stop",4))<0){
      //printf("write error\n");
    }
    close(remoteproc_state);
    remoteproc_state = -1;
    rpmsg_state = stepOne; //return to step one; processor should be stopped
    break;
  case stepThree://write firmware
    printf("step three\n");
    if(remoteproc_state < 0){
      remoteproc_state  = open(REMOTEPROC_FIRM,O_RDWR);
      return(rpmsg_state);
    }
    if((n=write(remoteproc_state,FIRMWARE,FIRMWARE_SIZE))<0){
      //printf("write error\n");
    }
    close(remoteproc_state);
    remoteproc_state = -1;
    rpmsg_state = stepFour; //start processor
    break;
  case stepFour://start
    printf("step four\n");
    if(remoteproc_state < 0){
      remoteproc_state  = open(REMOTEPROC_DEV,O_RDWR);
      return(rpmsg_state);
    }
    if((n=write(remoteproc_state,"start",5))<0){
      //printf("write error\n");
    }
    close(remoteproc_state);
    remoteproc_state = -1;
    rpmsg_state = stepFive;
    break;
  case stepFive://verify start
    printf("step five\n");
    if(remoteproc_state < 0){
      remoteproc_state  = open(REMOTEPROC_DEV,O_RDWR);
      return(rpmsg_state);
    }
    if((n = read(remoteproc_state,str,20)) > 0){
      if(strncmp("running",str,7) == 0){ //strings match
	//rpmsg_state = Initialized; //processor is correctly running
	rpmsg_state = stepSix;
	close(remoteproc_state);
	remoteproc_state = -1;
	init_command();
	return(rpmsg_state);
      }
    }
    break;
  case stepSix:
    printf("step six\n");
    if((*(CASSY) = open(CASSY_DEV,O_RDWR)) >= 0){
      flags=fcntl(*CASSY,F_GETFL,0);
      fcntl(*CASSY,F_SETFL,flags | O_NONBLOCK);
      rpmsg_state = Initialized;
    }
    break;
  case Initialized: //do nothing
    break;
  default:
    break;
  }
  return(rpmsg_state);
}

void listen(int CASSY){
  int k;
  if(buffer_state == 1) return;
  if ((plen = read(CASSY, buffer, RPMSG_BUF_SIZE)) > 0){
    //uncomment these lines if you want to see exactly what was received.  Good for tracing problems.
    //           for(k=0;k<plen;k++) printf("%c (%x) ",*(buffer+k),*(buffer+k));
    //printf("\n");
    buffer_state = 1;
    pbuffer = buffer; 
    return;
  }
  return;
}

unsigned char parse_the_message(unsigned char *action_state,unsigned char *R,unsigned char *N){
  static unsigned short command_state=0;
  static unsigned char *pReceived,*pNreceived;
  int k;
  unsigned char retval=0;
  
  *action_state = 'N';  
  if(buffer_state == 0) return(0);
  for(;plen > 0;plen--,pbuffer++){
    switch(command_state){
    case 0:
      if(*pbuffer == 'a') command_state = 1;
      break;
    case 1:
      if(*pbuffer == 'f') command_state = 2;
      break;
    case 2:
      *action_state = *pbuffer;
      command_state = 3;
      break;
    case 3:
      Ndata = *pbuffer;
      if(Ndata == 0){
	command_state = 0;
	retval = 1;
      }else command_state = 4;
      pNreceived = N;
      *pNreceived = 0;
      pReceived = R;
      break;
    case 4:
      //this will cause a hang if amount of data isn't equal to Ndata
      //need to figure a way to bail out of this if a new command arrives
      if(Ndata >= 1) {
	*pNreceived = *pNreceived + 1;
	*pReceived++ = *pbuffer;
	Ndata--;
      }
      if(Ndata == 0){
	command_state = 0;
	retval = 1;
      }
      break;
    default:
      command_state = 0;
      break;
    }
  }
  buffer_state = 0;
  return(retval);
}
void reset_command(void){
  Ncommand = 2;
  pcommand_str = &command_str[2];
  
}
void init_command(void){
  command_str[0] = 'a';
  command_str[1] = 'f';
  reset_command();
}
void compose_command(unsigned char b){
  *pcommand_str++ = b;
  Ncommand++;
}

unsigned char send_command(int CASSY){
  return(write(CASSY,command_str,Ncommand));
}


void act_on_command(unsigned char command_state){
  int k;

  for(k=0;k<Ncc;k++){
    if(command_state == cd[k]){ (*c[k])(); break;}
  }
}
void add_command_code(unsigned char x,void (*cfun)(void)){
  if(Ncc >= Maxc) return; //too many commands, exit without adding
  cd[Ncc] = x;
  c[Ncc] = cfun;
  Ncc++;
}

void add_action_code(unsigned char x,unsigned char (*afun)(void)){
  if(Naa >= Maxc) return; //too many commands, exit without adding
  ad[Naa] = x;
  a[Naa] = afun;
  Naa++;
}

unsigned char update_command_state(unsigned char action_state){
  int k;

  for(k=0;k<Naa;k++) if(action_state == ad[k]){ return((*a[k])());}
  return('N'); //this occurs if action_state isn't on the list
}
