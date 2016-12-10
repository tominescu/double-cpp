#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <string.h>
#include <signal.h>

using namespace std;

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

const int DEFAULT_PORT = 9990;
bool g_running = true;
int g_client_num = 0;


void Usage(const string& name) {
	cerr<<name<<" -p <listen_port>"<<endl;
}

int MainLoop(int epollfd, int listen_sock) {
	struct epoll_event *e = new epoll_event[g_client_num + 1];
	int ret = epoll_wait(epollfd, e, g_client_num + 1, -1);
	if (ret == -1) {
		cerr<"epoll_wait failed."<<endl;
		return -1;
	}
	if (ret == 0) {
		return 0;
	}
	for (int i = 0; i < ret; i++) {
		if (e[i].data.fd == listen_sock) {
			HandleListenSock(listen_sock);
		} else {
			HandleClientSock(e[i]);
		}
	}
}

void SignalHandler(int signum) {
	cerr<<"Server received signal "<<signum<<endl;
	g_running = false;
}

int main(int argc, char** argv) {
	int ch;
	int listen_port = -1;
	while (ch = getopt(argc, argv, "hp:") != -1) {
		switch (ch) {
			case 'h':
				Usage(argv[0]);
				return 0;
			case 'p':
				listen_port = atoi(optarg);
				break;
			default:
				Usage(argv[0]);
				return 1;
		}
	}

	if (listen_port <= 0 || listen_port >= 65536) {
		listen_port = DEFAULT_PORT;
	}

	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == -1) {
		handle_error("socket");
	}

	const int on = 1;
	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		handle_error("setsockopt");
	}
	if (setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) == -1) {
		handle_error("setsockopt");
	}

	int flag = fcntl(listen_sock, F_GETFL, 0);
	flag |= O_NONBLOCK;
	fcntl(listen_sock, F_SETFL, flag);

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(listen_port);

	if (bind(listen_sock, (struct sockaddr*)&sin, sizeof(sin)) == -1) {
		handle_error("bind");
	}

	if (listen(listen_sock, 1024) == -1) {
		handle_error("listen");
	}

	int epollfd = epoll_create(256);
	if (epollfd == -1) {
		handle_error("epoll_create");
	}

	struct epoll_event ev;
	ev.data.fd = listen_sock;
	ev.events = EPOLLIN | EPOLLET;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		handle_error("epoll_ctl");
	}

	cerr<<"Server listen on 0.0.0.0:"<<listen_port<<endl;
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, &SignalHandler);
	signal(SIGINT, &SignalHandler);
	int ret = 0;
	while (g_running) {
		ret = MainLoop(epollfd, listen_sock);
		if (ret != 0) {
			break;
		}
	}
	cerr<<"Server exit with code:"<<ret<<endl;
	return -1;
}
