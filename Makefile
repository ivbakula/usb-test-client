CC=gcc
LD=-lusb-1.0 -lm
SRC=usb-comm.c

all:
	$(CC) $(LD) $(SRC) -o usb-comm
