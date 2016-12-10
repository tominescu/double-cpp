#include "package.h"
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

using namespace std;

Package::Package() {
    buf_ = (char*)malloc(MAX_PACKAGE_SIZE);
    read_offset_ = 0;
    write_offset_ = 0;
}

Package::Package(const Package& p) {
    buf_ = (char*)malloc(MAX_PACKAGE_SIZE);
    memcpy(buf_, p.buf_, MAX_PACKAGE_SIZE);
    read_offset_ = p.read_offset_;
    write_offset_ = p.write_offset_;
}

Package& Package::operator=(const Package& p) {
    buf_ = (char*)malloc(MAX_PACKAGE_SIZE);
    memcpy(buf_, p.buf_, MAX_PACKAGE_SIZE);
    read_offset_ = p.read_offset_;
    write_offset_ = p.write_offset_;
}

Package::~Package() {
    if (buf_ != NULL) {
        free(buf_);
    }
    read_offset_ = 0;
    write_offset_ = 0;
}

int Package::ReadSock(int sock) {
    if (read_offset_ == write_offset_ && read_offset_ > 0) {
        read_offset_ = 0;
        write_offset_ = 0;
    }
    ssize_t nread = recv(sock, buf_ + read_offset_, MAX_PACKAGE_SIZE - read_offset_, 0);
    if (nread == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        } else {
            string prefix = client_ip_ + " recv";
            perror(prefix.c_str());
            return -1;
        }
    } else if (nread == 0) {
        cerr<<client_ip_<<" closed connection."<<endl;
        return -1;
    } else {
        read_offset_ += nread;
    }
    return nread;
}

int Package::WriteSock(int sock) {
    if (write_offset_ == read_offset_) {
        return 0;
    }
    ssize_t nwrite = send(sock, buf_ + write_offset_, read_offset_ - write_offset_, 0);
    if (nwrite == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        } else {
            string prefix = client_ip_ + " recv";
            perror(prefix.c_str());
            return -1;
        }
    } else if (nwrite == 0) {
        return 0;
    } else {
        write_offset_ += nwrite;
    }
    return nwrite;
}

void Package::SetClientIP(const string& ip) {
    client_ip_ = ip;
}

const string& Package::GetClientIP() {
    return client_ip_;
}
