#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <exception>
#include <stdexcept>
#include <string>
#include <cstring>
#include "Exception.h"
#include "MySocket.h"
#include "Packet.h"
#include <pthread.h>
#include <unistd.h>


using std::string;

typedef enum {
    TIME_OK,
    NAME_OK,
    LIST_OK,
    POST_OK,
    POST_UNKNOWN_NUM,
    POST_UNCLEAN_DISCONNECT,
    INST_MSG,
    ERROR
} Msg_type;

class Client : public MySocket
{
private:
    string _server_addr;
    int _server_port;
    int _qid;

    pthread_t _listenThread;
    pthread_t _feedbackThread;
public:
    Client() = default;
    void Connect(string addr, int port);
    void Start();
    ~Client();
};


void* ListenService(void* _pt);
void* FeedbackService(void* _pt);
Msg_type ParseResponse(string& response, string& content, string& sender, string& receiver);

#endif