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

using namespace std;

const char* DEFAULT_ADDR = "127.0.0.1";
const int DEFAULT_PORT = 9990;

void Usage(const string& name) {
    cerr<<name<<" -p <server_port> server_ip"<<endl;
}

void MainLoop(int sock) {
    string line;
    char buf[1024];
    while (true) {
        cout<<"> ";
        if (!getline(cin, line)) {
            break;
        }
        ssize_t nwrite = send(sock, line.c_str(), line.size(), 0);
        if (nwrite == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                cerr<<"Server busy, send nothing."<<endl;
            } else {
                perror("send");
                break;
            }
        }
        if (nwrite < (ssize_t)line.size()) {
            cerr<<"Server busy, send ["<<line.substr(0,nwrite)<<"]"<<endl;
        }

        ssize_t nread = recv(sock, buf, sizeof(buf), 0);
        if (nread == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                break;
            }
        } else if (nread == 0) {
            cerr<<"Lost connection from server."<<endl;
            break;
        } else {
            cout<<string(buf, nread)<<endl;
        }
    }
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

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        handle_error("socket");
    }

    sockaddr_in sin;
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    if (inet_aton(remote_addr.c_str(), &sin.sin_addr) == -1) {
        handle_error("inet_aton");
    }
    sin.sin_port = htons(remote_port);

    if (connect(sock, (sockaddr*)&sin, sizeof(sin)) == -1) {
        handle_error("connect");
    }
    cerr<<"Connected to "<<remote_addr<<":"<<remote_port<<endl;
    
    SetNonBlock(sock);
    SetSockBufSize(sock, 65536, 65536);

    MainLoop(sock);
    close(sock);
    cerr<<"Disonnected to "<<remote_addr<<":"<<remote_port<<endl;
    return 0;
}
