HEADERS = aux.h macros.h
OBJECTS = main.o aux.o

# Parameters
CC = gcc
CFLAGS = -Wall -c

USER = anonymous
PWD = anonymous@
HOST = ftp://netlab1.fe.up.pt
PATH =

default: download

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

download: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f download
