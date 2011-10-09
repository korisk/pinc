CC=gcc
CFLAGS=-I.  -O3 -D_GNU_SOURCE
LDFLAGS=-lpthread

all: check.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o pinc check.o 

clean:
	rm ./*.o ./pinc
