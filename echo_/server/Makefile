CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lpthread

all: echoserver

echoserver: echoserver.o csapp.o
    $(CC) $(CFLAGS) -o echoserver echoserver.o csapp.o $(LIBS)

echoserver.o: echoserver.c csapp.h
    $(CC) $(CFLAGS) -c echoserver.c

csapp.o: ../csapp.c ../csapp.h
    $(CC) $(CFLAGS) -c ../csapp.c

clean:
    rm -f *.o echoserver
