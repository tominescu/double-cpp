CC = g++
oFLAGS = -std=c++11

.PHONEY: all clean

all: epoll-server client

epoll-server: epoll-server.o package.o common.o
	$(CC) $(oFLAGS) -o $@ $^ 

client: client.o common.o
	$(CC) $(oFLAGS) -o $@ $^ 

.cpp.o:
	$(CC) $(oFLAGS) -c -o $@ $^ 

clean:
	rm -rf epoll-server client *.o
