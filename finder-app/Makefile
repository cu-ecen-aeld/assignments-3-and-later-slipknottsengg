CC=$(CROSS_COMPILE)gcc
CFLAGS+=-Wall -Wextra

all: writer

maker: writer.c
	$(CC) -o writer -c writer.c

clean:
	rm -rf *.o writer

.PHONY: all clean
