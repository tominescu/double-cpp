#ifndef COMMON_H_
#define COMMON_H_
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define handle_error(msg) \
    do { \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while(0) 

void SetNonBlock(int fd);
void SetSockBufSize(int sock, size_t recv_buf_size, size_t send_buf_size);

#endif
