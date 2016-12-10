#include "package.h"
#include "common.h"

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

const int DEFAULT_PORT = 9990;
bool g_running = true;
int g_client_num = 0;
unordered_map<int, Package> g_client_data;

void Usage(const string& name) {
    cerr<<name<<" -p <listen_port>"<<endl;
}

void HandleListenSock(int epollfd, int listen_sock) {
    while(true) {
        sockaddr_in sin;
        socklen_t socklen = sizeof(sockaddr);
        int clientsock = accept(listen_sock, (sockaddr*)&sin, &socklen);
        if (clientsock == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            handle_error("accept");
        }
        stringstream ss;
        ss << inet_ntoa(sin.sin_addr) << ":" << ntohs(sin.sin_port);
        string client_addr = ss.str();
        cerr<<"New client from "<<client_addr<<endl;
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
    cerr<<"close client "<<g_client_data[sock].GetClientIP()<<endl;
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
            cerr<<"read "<<ret<< " bytes from " << client_pack.GetClientIP() << endl;
            if (ret == -1) {
                CloseClientSock(epollfd, client_sock);
                break;
            }
            ret = client_pack.WriteSock(client_sock);
            cerr<<"write "<<ret<< " bytes to" << client_pack.GetClientIP() << endl;
            if (ret == -1) {
                CloseClientSock(epollfd, client_sock);
                break;
            } else if (ret == 0) {
                break;
            }
        }
    } else {
        cerr<<client_pack.GetClientIP()<<" other event happens"<<endl;
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
    cerr<<"Server received signal "<<signum<<endl;
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

    SetNonBlock(listen_sock);

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(listen_port);

    if (bind(listen_sock, (sockaddr*)&sin, sizeof(sin)) == -1) {
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
