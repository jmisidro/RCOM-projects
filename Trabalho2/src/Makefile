HEADERS = app.h macros.h
OBJECTS = main.o app.o

# Parameters
CC = gcc
CFLAGS = -Wall -c

USER = rcom
PASS = rcom
HOST = netlab1.fe.up.pt
URL_PATH = files/pic1.jpg
FILE = pic1.jpg

default: download

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

download: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

run_download:
	./download ftp://$(USER):$(PASS)@$(HOST)/$(URL_PATH)

clean:
	-rm -f $(OBJECTS)
	-rm -f download
	-rm -f $(FILE)
