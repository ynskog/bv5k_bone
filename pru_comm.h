#ifndef __pru_comm__
#define __pru_comm__ 1
#include <pru_rpmsg.h>

#define Maxc 20


enum rpmsg_state_type {stepOne, stepTwo, stepThree,Initialized,stepFour,stepFive};

//function prototypes
unsigned short rpmsg_init(void);
unsigned char parse_the_message(unsigned char *action_state,unsigned char *R,unsigned char *N);
//unsigned short send(char *buf,int N);
void reset_command(void);
void compose_command(unsigned char b);
unsigned char send_command(void);
void add_command_code(unsigned char x,unsigned short (*cfun)(void));
void act_on_command(unsigned char command_state);
unsigned char update_command_state(unsigned char action_state);
void listen(void);
void add_action_code(unsigned char x,unsigned char (*afun)(void));
//externally needed variables
extern unsigned char rpmsg_errno;
extern unsigned char Nvalue;


#endif
