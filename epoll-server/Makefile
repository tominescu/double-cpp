CC = g++
CFLAGS = -std=c++11 -g

.PHONEY: all clean

all: epoll-server client

epoll-server: epoll-server.o package.o common.o
	$(CC) $(CFLAGS) -o $@ $^ 

client: client.o common.o
	$(CC) $(CFLAGS) -o $@ $^ 

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $^ 

clean:
	rm -rf epoll-server client *.o
