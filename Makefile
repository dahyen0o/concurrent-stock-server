CC = gcc
CFLAGS=-O2 -Wall
LDLIBS = -lpthread

all: multiclient stockserver

multiclient: multiclient.c csapp.c csapp.h
stockserver: stockserver.c csapp.c csapp.h

clean:
	rm -rf *~ multiclient stockserver *.o
