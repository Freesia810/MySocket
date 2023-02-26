#ifndef __MYSOCKET_H__
#define __MYSOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <exception>
#include <stdexcept>
#include <unistd.h>
#include "Exception.h"

class MySocket
{
protected:
    int socketFd;
public:
    MySocket() = default;
    void Initialize(){
        socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socketFd == -1){
            throw MySocketException(INITIALIZATION_FAILED);
        }
    };
    ~MySocket() {}
};

#endif