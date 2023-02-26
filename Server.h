#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include "Exception.h"
#include "MySocket.h"
#include <pthread.h>

using std::string;

typedef enum {
    REQU_TIME,
    REQU_NAME,
    REQU_LIST,
    REQU_POST,
    ERROR
} Msg_type;


struct client_info{
    int clientFd;
    string addr;
    int port;
};

class Server : public MySocket
{
private:
    string _addr;
    int _port;
public:
    Server() = default;
    void Listen(int port, string addr = "");
    void Start();
    ~Server();
};

void* ClientService(void* info_pt);
Msg_type ParseRequest(string& request, int& targetClientNum, string& msg);
#endif