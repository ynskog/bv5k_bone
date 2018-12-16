#ifndef __arm_pru_comm__
#define __arm_pru_comm__ 1

#define RPMSG_BUF_SIZE 512
#define Maxc 20

enum rpmsg_state_type {stepOne, stepTwo, stepThree,Initialized,stepFour,stepFive,stepSix};

//function prototypes ... create arm_pru_comm.h
void init_command(void);
unsigned short rpmsg_init(int *CASSY);
void listen(int CASSY);
unsigned char parse_the_message(unsigned char *action_state,unsigned char *R,unsigned char *N);
void reset_command(void);
void compose_command(unsigned char b);
unsigned char send_command(int CASSY);
void add_command_code(unsigned char x,void (*cfun)(void));
void add_action_code(unsigned char x,unsigned char (*afun)(void));
void act_on_command(unsigned char command_state);
unsigned char parse_the_message(unsigned char *action_state,unsigned char *R,unsigned char *N);
unsigned char update_command_state(unsigned char action_state);
void rpmsg_deinit(int CASSY);

#endif
