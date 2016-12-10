#include "common.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

void SetNonBlock(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);
	flag |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flag);
}

void SetSockBufSize(int sock, size_t recv_buf_size, size_t send_buf_size) {
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(recv_buf_size)) == -1) {
        handle_error("setsockopt");
    }
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(send_buf_size)) == -1) {
        handle_error("setsockopt");
    }
}


