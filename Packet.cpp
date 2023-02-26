#include "Packet.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <exception>
#include <stdexcept>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sstream>
#include <iostream>
#include <string>


Packet::Packet(/* args */)
{
    _buffer = NULL;
    _payload_size = 0;
}

Packet::~Packet()
{
    if(_buffer != NULL){
        delete[] _buffer;
    }
}


void Packet::loadBuffer(string str){
    _payload_size = strlen(str.c_str());
    _buffer = new char[_payload_size];
    strcpy(_buffer, str.c_str());
    _buffer[_payload_size - 1] = (char)Magic_Number;
}


void Packet::loadTimeRequest(){
    loadBuffer("REQU TIME\n");
}
void Packet::loadTimeResponse(string time){
    string tmp = "RESP TIME OK\n" + time;
    loadBuffer(tmp);
}



void Packet::loadNameRequest(){
    loadBuffer("REQU NAME\n");
}
void Packet::loadNameResponse(string name){
    string tmp = "RESP NAME OK\n" + name;
    loadBuffer(tmp);
}


void Packet::loadClientListRequest(){
    loadBuffer("REQU LIST\n");
}
void Packet::loadClientListResponse(string listStr){
    string tmp = "RESP LIST OK\n" + listStr;
    loadBuffer(tmp);
}



void Packet::loadMessageRequest(int receiver, string content){
    string tmp = "REQU POST\nTo: " + std::to_string(receiver) + "\n" + content;
    loadBuffer(tmp);
}

void Packet::loadMessageResponse(int type){
    switch (type)
    {
    case 1:
        loadBuffer("RESP POST OK\n");
        break;
    case 0:
        loadBuffer("RESP POST UNKNOWN NUM\n");
        break;
    case 2:
        loadBuffer("RESP POST UNCLEAN DISCONNECT\n");
        break;
    default:
        break;
    }
}
void Packet::loadMessageInstruction(int sender, int receiver, string content){
    string tmp = "INST MSG\nFrom: " + std::to_string(sender) + "\nTo: " + std::to_string(receiver) +"\n" +content;
    loadBuffer(tmp);
}


void Packet::Send(int socketFd){
    if(_payload_size < MAX_PAYLOAD){
        send(socketFd, _buffer, _payload_size, 0);
    }
    else{
        int pkg_num = _payload_size / MAX_PAYLOAD + 1;
        int remaining_size = _payload_size - (pkg_num - 1) * MAX_PAYLOAD;
        for(int i = 0; i < pkg_num; i++){
            if(i != pkg_num - 1){
                send(socketFd, &_buffer[MAX_PAYLOAD * pkg_num], MAX_PAYLOAD, 0);
            }
            else{
                send(socketFd, &_buffer[MAX_PAYLOAD * pkg_num], remaining_size, 0);
            }
        }
    }
}

void Packet::Receive(int socketFd){
    _payload_size = 0;

    string temp = "";
    while(true){
        char buffer[MAX_PAYLOAD];
        int thisSize = recv(socketFd, buffer, MAX_PAYLOAD, 0);
        if(thisSize <= 0){
            _payload_size = -1;
            _buffer = NULL;
            return;
        }
        if(thisSize < MAX_PAYLOAD){
            //这个buffer不满
            unsigned char test_num = buffer[thisSize - 1];
            if(test_num == Magic_Number){
                //收到end标志,结束接收
                buffer[thisSize - 1] = '\0';
                temp += string(buffer);
                break;
            }
        }
        else{
            //数据可能超过一包
            if(buffer[MAX_PAYLOAD - 1] == Magic_Number){
                buffer[MAX_PAYLOAD - 1] = '\0';
                temp += string(buffer);
                break;
            }
            else{
                char last = buffer[MAX_PAYLOAD - 1];
                buffer[MAX_PAYLOAD - 1] = '\0';
                temp += string(buffer) + last;
            }
        }
    }

    _payload_size = temp.length() + 1;
    if(_buffer != NULL){
        delete[] _buffer;
    }
    _buffer = new char[_payload_size];
    strcpy(_buffer, temp.c_str());
}


void Packet::getMessages(std::vector<string>& messages){
    string raw = getBuffer();

    std::istringstream iss(raw);    // 输入流
    string token;            // 接收缓冲区
    while (std::getline(iss, token, (char)Magic_Number))    // 以split为分隔符
    {
        messages.push_back(token);
    }
}