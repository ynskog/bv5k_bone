NAME:= hello
$(NAME): $(NAME).c arm_comm.c arm_comm.h
	gcc $(NAME).c arm_comm.c -o $(NAME)

PRU_RPMSG_ROOT:= /usr/lib/ti/pru-software-support-package/
PRU_CGT_ROOT:= /usr/share/ti/cgt-pru/
PRU_INCLUDE:= --include_path=$(PRU_CGT_ROOT)include --include_path=$(PRU_RPMSG_ROOT)include/ --include_path=$(PRU_RPMSG_ROOT)include/am335x/
CODE_ROOT:=/home/ys/$(NAME)/
PRU0_ROOT:= $(CODE_ROOT)pru0/
PRU1_ROOT:= $(CODE_ROOT)pru1/
LINKER_CMD_FILE:= $(NAME).cmd
PRU_TOOLS:=/usr/bin/

CFLAGS=-v3 -O2 --endian=little --hardware_mac=on
LDFLAGS=--reread_libs --warn_sections --library=$(PRU_CGT_ROOT)lib/libc.a --library=$(PRU_RPMSG_ROOT)lib/rpmsg_lib.lib 

$(PRU0_ROOT)pru_comm.object: $(CODE_ROOT)pru_comm.c
	$(PRU_TOOLS)clpru $(CFLAGS) $(PRU_INCLUDE) -ppd -ppa -fe $(PRU0_ROOT)pru_comm.object $(CODE_ROOT)pru_comm.c -D PRU0

$(PRU0_ROOT)main.object: $(CODE_ROOT)main.c
	$(PRU_TOOLS)clpru $(CFLAGS) $(PRU_INCLUDE) -ppd -ppa -fe $(PRU0_ROOT)main.object $(CODE_ROOT)main.c -D PRU0

am335x-pru0-fw: $(PRU0_ROOT)main.object $(PRU0_ROOT)pru_comm.object
	$(PRU_TOOLS)clpru -z $(LINKER_CMD_FILE) -o $(PRU0_ROOT)am335x-pru0-fw $(PRU0_ROOT)main.object $(PRU0_ROOT)pru_comm.object $(LDFLAGS)

$(PRU1_ROOT)pru_comm.object: $(CODE_ROOT)pru_comm.c
	$(PRU_TOOLS)clpru $(CFLAGS) $(PRU_INCLUDE) -ppd -ppa -fe $(PRU1_ROOT)pru_comm.object $(CODE_ROOT)pru_comm.c -D PRU1

$(PRU1_ROOT)main.object: $(CODE_ROOT)main.c
	$(PRU_TOOLS)clpru $(CFLAGS) $(PRU_INCLUDE) -ppd -ppa -fe $(PRU1_ROOT)main.object $(CODE_ROOT)main.c -D PRU1

am335x-pru1-fw: $(PRU1_ROOT)main.object $(PRU1_ROOT)pru_comm.object
	$(PRU_TOOLS)clpru -z $(LINKER_CMD_FILE) -o $(PRU1_ROOT)am335x-pru1-fw $(PRU1_ROOT)main.object $(PRU1_ROOT)pru_comm.object $(LDFLAGS)

install: am335x-pru0-fw am335x-pru1-fw
	cp $(PRU0_ROOT)/am335x-pru0-fw /lib/firmware
	cp $(PRU1_ROOT)/am335x-pru1-fw /lib/firmware

clean:
	rm $(PRU0_ROOT)/am335x-pru0-fw -f
	rm $(PRU1_ROOT)/am335x-pru1-fw -f
	rm $(PRU0_ROOT)/*.object -f
	rm $(PRU1_ROOT)/*.object -f
	rm *.pp -f

