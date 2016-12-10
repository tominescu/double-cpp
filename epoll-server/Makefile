CC = g++
CPPFLAGS = -std=c++11

.PHONEY: all clean

all: epoll-server client

epoll-server: epoll-server.o package.o common.o
	$(CC) $(CPPFLAGS) -o $@ $^ 

client: client.o common.o
	$(CC) $(CPPFLAGS) -o $@ $^ 

clean:
	rm -rf epoll-server client *.o
