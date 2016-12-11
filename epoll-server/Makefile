CC = g++
CFLAGS = -std=c++11 -g

.PHONEY: all clean

all: epoll-server telnet-client

epoll-server: epoll-server.o package.o common.o
	$(CC) $(CFLAGS) -o $@ $^ 

telnet-client: telnet-client.o common.o
	$(CC) $(CFLAGS) -o $@ $^ 

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $^ 

clean:
	rm -rf epoll-server telnet-client *.o
