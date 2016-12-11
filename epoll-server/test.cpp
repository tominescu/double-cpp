#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char** argv) {
    if (argc != 2) {
        exit(1);
    }
    cerr<<"argv[1]:"<<argv[1]<<endl;

    addrinfo hints, *res, *rp;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;

    int ret = getaddrinfo(argv[1], "9990", &hints, &res);
    if (ret != 0) {
        cerr<<"getaddrinfo:"<<gai_strerror(ret)<<endl;
        exit(1);
    }
    char buf[INET6_ADDRSTRLEN];
    bzero(buf, sizeof(buf));

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        switch (rp->ai_family) {
            case PF_INET:
                if (inet_ntop(AF_INET, &((sockaddr_in*)rp->ai_addr)->sin_addr, buf, sizeof(buf)) == NULL) {
                    perror("inet_ntop");
                    exit(1);
                }
                break;
            case PF_INET6:
                if (inet_ntop(AF_INET6, &((sockaddr_in6*)rp->ai_addr)->sin6_addr, buf, sizeof(buf)) == NULL) {
                    perror("inet_ntop");
                    exit(1);
                }
                break;
            default:
                cerr<<"unknown family:"<<rp->ai_family<<endl;
                break;
        }
        cerr<<"remote addr: "<<buf<<"cname: "<<rp->ai_canonname<<endl;
    }
    freeaddrinfo(res);
    return 0;

}
