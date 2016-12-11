#include "common.h"

#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sstream>

using namespace std;

const char* DEFAULT_ADDR = "127.0.0.1";
const int DEFAULT_PORT = 9990;
char g_buf[1024];

void Usage(const string& name) {
    cerr<<name<<" -p <server_port> server_ip"<<endl;
}

bool MainLoop(int sock) {
    fd_set rdfds;
    FD_ZERO(&rdfds);
    FD_SET(0, &rdfds);
    FD_SET(sock, &rdfds);
    int ret = select(sock + 1, &rdfds, NULL, NULL, NULL);
    if (ret <= 0) {
        handle_error("select");
    }
    string line;
    if (FD_ISSET(0, &rdfds)) {
        if(getline(cin, line)) {
            line += '\n';
            ssize_t nwrite = send(sock, line.c_str(), line.size(), 0);
            if (nwrite == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    cerr<<"Server busy, send nothing."<<endl;
                } else {
                    perror("send");
                    return false;
                }
            }
            if (nwrite < (ssize_t)line.size()) {
                cerr<<"Server busy, send ["<<line.substr(0,nwrite)<<"]"<<endl;
            }
        } else {
            return false;
        }
    }

    if (FD_ISSET(sock, &rdfds)) {
        ssize_t nread = recv(sock, g_buf, sizeof(g_buf), 0);
        if (nread == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return true;
            } else {
                return false;
            }
        } else if (nread == 0) {
            cerr<<"Lost connection from server."<<endl;
            return false;
        } else {
            cout<<string(g_buf, nread);
        }
   
    }
    return true;
}

int main(int argc, char** argv) {
    int ch;
    int remote_port = -1;
    string remote_addr;
    while ((ch = getopt(argc, argv, "hp:")) != -1) {
        switch (ch) {
            case 'h':
                Usage(argv[0]);
                exit(0);
            case 'p':
                remote_port = atoi(optarg);
                break;
            default:
                Usage(argv[0]);
                exit(1);
        }
    }

    if (remote_port <= 0 || remote_port >= 65536) {
        remote_port = DEFAULT_PORT;
    }

    if (optind == argc - 1) {
        remote_addr = argv[optind];
    } else if (optind == argc) {
        remote_addr = DEFAULT_ADDR;
    } else {
        Usage(argv[0]);
        return -1;
    }

    /* resove address and connect */
    addrinfo hints, *res, *rp;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    char remote_port_str[8];
    snprintf(remote_port_str, sizeof(remote_port_str), "%d", remote_port);

    int ret = getaddrinfo(remote_addr.c_str(), remote_port_str, &hints, &res);
    if (ret != 0) {
        cerr<<"getaddrinfo:"<<gai_strerror(ret)<<endl;
        return -1;
    }

    char buf[INET6_ADDRSTRLEN];
    bzero(buf, sizeof(buf));

    int sock = -1;

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        switch (rp->ai_family) {
            case AF_INET:
                if (inet_ntop(AF_INET, &((sockaddr_in*)rp->ai_addr)->sin_addr, buf, sizeof(buf)) == NULL) {
                    perror("inet_ntop");
                    exit(1);
                }
                break;
            case AF_INET6:
                if (inet_ntop(AF_INET6, &((sockaddr_in6*)rp->ai_addr)->sin6_addr, buf, sizeof(buf)) == NULL) {
                    perror("inet_ntop");
                    exit(1);
                }
                break;
            default:
                cerr<<"unknown family:"<<rp->ai_family<<endl;
                break;
        }
        cerr<<"Trying "<<buf<<"..."<<endl;
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) {
            perror("socket");
            continue;
        }

        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == -1) {
            perror("connect");
            continue;
        }
        cerr<<"Connected to "<<remote_addr<<":"<<remote_port<<endl;
        cerr<<"Press Ctrl + D to quit."<<endl;
        break;
    }
    freeaddrinfo(res);
    if (rp == NULL) {
        cerr<<"Can't connect to "<<remote_addr<<":"<<remote_port<<endl;
        exit(1);
    }
    
    SetNonBlock(sock);
    SetSockBufSize(sock, 65536, 65536);

    while(MainLoop(sock));
    close(sock);
    cerr<<"Disonnected to "<<buf<<":"<<remote_port<<endl;
    return 0;
}
