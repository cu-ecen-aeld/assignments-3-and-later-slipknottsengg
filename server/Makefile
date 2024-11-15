CC=$(CROSS_COMPILE)gcc
CFLAGS+= -g -Wall -Wextra

all: aesdsocket

aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.o

aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) -o aesdsocket.o -c aesdsocket.c

clean:
	rm -rf *.o aesdsocket

.PHONY: all clean
