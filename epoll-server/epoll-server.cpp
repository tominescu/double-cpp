#include "package.h"
#include "common.h"
#include "log.h"

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
#include <unordered_map>
#include <sstream>

using namespace std;

//#define DEBUG // enable debug log

const int DEFAULT_PORT = 9990;
bool g_running = true;
int g_client_num = 0;
unordered_map<int, Package> g_client_data;

void Usage(const string& name) {
    cerr<<name<<" -p <listen_port>"<<endl;
}

void HandleListenSock(int epollfd, int listen_sock) {
    while(true) {
        sockaddr_in6 sin6;
        socklen_t socklen = sizeof(sockaddr_in6);
        int clientsock = accept(listen_sock, (sockaddr*)&sin6, &socklen);
        if (clientsock == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            handle_error("accept");
        }
        stringstream ss;
        char buf[128];
        bzero(buf, sizeof(buf));
        string client_addr;
        if (inet_ntop(PF_INET6, (const void*)&sin6.sin6_addr, buf, sizeof(buf)) != NULL) {
            ss << buf << ":" << ntohs(sin6.sin6_port);
            client_addr = ss.str();
        }
        INFO_LOG("New client from %s", client_addr.c_str());
        SetNonBlock(clientsock);
        SetSockBufSize(clientsock, 65536, 65536);
        epoll_event ev;
        ev.data.fd = clientsock;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock, &ev) == -1) {
            handle_error("client sock epoll_ctl");
        }
        g_client_num ++;
        g_client_data[clientsock].SetClientIP(client_addr);
    }
}

void CloseClientSock(int epollfd, int sock) {
    INFO_LOG("Close client %s", g_client_data[sock].GetClientIP().c_str());
    close(sock);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, sock, NULL);
    g_client_num --;
    g_client_data.erase(sock);
}

void HandleClientSock(int epollfd, const epoll_event& event) {
    int ret = 0;
    int client_sock = event.data.fd;
    Package& client_pack = g_client_data[client_sock];
    if (event.events & EPOLLIN || event.events & EPOLLOUT) {
        while (true) {
            ret = client_pack.ReadSock(client_sock);
            INFO_LOG("Read %d bytes from %s", ret, client_pack.GetClientIP().c_str());
            if (ret == -1) {
                CloseClientSock(epollfd, client_sock);
                break;
            }
            ret = client_pack.WriteSock(client_sock);
            INFO_LOG("Write %d bytes to %s", ret, client_pack.GetClientIP().c_str());
            if (ret == -1) {
                CloseClientSock(epollfd, client_sock);
                break;
            } else if (ret == 0) {
                break;
            }
        }
    } else {
        ERR_LOG("%s other event happens", client_pack.GetClientIP().c_str());
        CloseClientSock(epollfd, client_sock);
    }
}

int MainLoop(int epollfd, int listen_sock) {
    epoll_event *e = new epoll_event[g_client_num + 1];
    int ret = epoll_wait(epollfd, e, g_client_num + 1, -1);
    if (ret == -1) {
        perror("epoll_wait");
        return 0;
    }
    for (int i = 0; i < ret; i++) {
        if (e[i].data.fd == listen_sock) {
            HandleListenSock(epollfd, listen_sock);
        } else {
            HandleClientSock(epollfd, e[i]);
        }
    }
    delete[] e;
    return 0;
}

void SignalHandler(int signum) {
    WARN_LOG("Server received signal %d", signum);
    g_running = false;
}

int main(int argc, char** argv) {
    int ch;
    int listen_port = -1;
    while ((ch = getopt(argc, argv, "hp:")) != -1) {
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

    int listen_sock = socket(PF_INET6, SOCK_STREAM, 0);
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

    SetNonBlock(listen_sock);

    sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = PF_INET6;
    sin6.sin6_addr = in6addr_any;
    sin6.sin6_port = htons(listen_port);

    if (bind(listen_sock, (sockaddr*)&sin6, sizeof(sin6)) == -1) {
        handle_error("bind");
    }

    if (listen(listen_sock, 1024) == -1) {
        handle_error("listen");
    }

    int epollfd = epoll_create(256);
    if (epollfd == -1) {
        handle_error("epoll_create");
    }

    epoll_event ev;
    ev.data.fd = listen_sock;
    ev.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        handle_error("epoll_ctl");
    }

    INFO_LOG("Server listen on :::%d", listen_port);
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
    INFO_LOG("Server exit with code:%d", ret);
    return -1;
}
