#ifndef PACKAGE_H_
#define PACKAGE_H_

#include <stdlib.h>
#include <iostream>

const int MAX_PACKAGE_SIZE = 4096;

class Package{
public:
    Package();
    Package(const Package& p);
    Package& operator=(const Package& p);
    ~Package();

    int ReadSock(int sock);
    int WriteSock(int sock);
    void SetClientIP(const std::string& ip);
    const std::string& GetClientIP();
private:
    char* buf_;
    ssize_t read_offset_;
    ssize_t write_offset_;
    std::string client_ip_;
};

#endif
