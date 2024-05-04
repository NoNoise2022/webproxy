CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lpthread

all: echoclient

echoclient: echoclient.o csapp.o
    $(CC) $(CFLAGS) -o echoclient echoclient.o csapp.o $(LIBS)

echoclient.o: echoclient.c csapp.h
    $(CC) $(CFLAGS) -c echoclient.c

csapp.o: ../csapp.c ../csapp.h
    $(CC) $(CFLAGS) -c ../csapp.c

clean:
    rm -f *.o echoclient
