#ifndef __PACKET_H__
#define __PACKET_H__

#include <iostream>
#include <string>
#include <vector>

using std::string;
class Packet
{
private:
    const int MAX_PAYLOAD{1024};
    const unsigned char Magic_Number = 0xCB;
    char* _buffer;
    int _payload_size;
    void loadBuffer(string str);
    inline const char* getBuffer(){ return _buffer;};
public:
    Packet();
    ~Packet();

    void loadTimeRequest();
    void loadTimeResponse(string time);

    void loadNameRequest();
    void loadNameResponse(string name);

    void loadClientListRequest();
    void loadClientListResponse(string listStr);

    void loadMessageRequest(int receiver, string content);
    void loadMessageResponse(int type);
    void loadMessageInstruction(int sender, int receiver, string content);


    void Send(int socketFd);
    void Receive(int socketFd);
    void getMessages(std::vector<string>& messages);
    inline int getPayloadSize() {return _payload_size;};
};

#endif