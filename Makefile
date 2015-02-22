CFLAGS=$(shell pkg-config --cflags x11) -std=gnu99 -O3 -Wall
LIBS=$(shell pkg-config --libs x11)

all:
	gcc ${CFLAGS} ${LIBS} ctrlesc.c -o ctrlesc
