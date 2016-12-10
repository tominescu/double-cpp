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

void Usage(const string& name) {
    cerr<<name<<" -p <server_port> server_ip"<<endl;
}

bool GetIP(const string& remote_addr, string& ip) {
    sockaddr_in sin;
    int ret = inet_pton(AF_INET, remote_addr.c_str(), &sin);
    if (ret > 0) {
        /* remote addr is an ip address */
        ip = remote_addr;
        return true;
    }
    hostent *he;
    in_addr **in_addr_list;
    he = gethostbyname(remote_addr.c_str());
    if (he == NULL) {
        return false;
    }
    in_addr_list = (in_addr**)he->h_addr_list;
    for (int i = 0; in_addr_list[i] != NULL; i++) {
        ip = inet_ntoa(*in_addr_list[i]);
        ret = inet_pton(AF_INET, ip.c_str(), &sin);
        if (ret > 0) {
            return true;
        }
    }
    return false;
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

    string remote_ip;
    if (!GetIP(remote_addr, remote_ip)) {
        cerr<<"can't resolve name of host "<<remote_addr<<endl;
        return -1;
    }

    sockaddr_in sin;
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    if (inet_aton(remote_ip.c_str(), &sin.sin_addr) == -1) {
        handle_error("inet_aton");
    }
    sin.sin_port = htons(remote_port);

    if (connect(sock, (sockaddr*)&sin, sizeof(sin)) == -1) {
        stringstream ss;
        ss << "connect to "<<remote_ip<<":"<<remote_port;
        handle_error(ss.str().c_str());
    }
    cerr<<"Connected to "<<remote_ip<<":"<<remote_port<<endl;
    
    SetNonBlock(sock);
    SetSockBufSize(sock, 65536, 65536);

    MainLoop(sock);
    close(sock);
    cerr<<"Disonnected to "<<remote_ip<<":"<<remote_port<<endl;
    return 0;
}
