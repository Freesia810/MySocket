#include "Client.h"
#include "Packet.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <iostream>
#include <semaphore.h>
#include <queue>
#include <vector>

std::queue<std::string> msgQueue;

extern bool isQuit;
bool isConnected = true;

void Client::Connect(string addr, int port){
    _server_addr = addr;
    _server_port = port;

    sockaddr_in addrin{AF_INET, htons(port), {inet_addr(addr.c_str())}};
    memset(&addrin.sin_zero, 0, sizeof(addrin.sin_zero));

    if(connect(socketFd, (sockaddr*)&addrin, sizeof(addrin)) != 0){
        throw MySocketException(CONNECTION_FAILED);
    }
    isConnected = true;
    std::cout << "Connect to " << _server_addr << ":" << std::to_string(_server_port) << std::endl;
}



void Client::Start(){
    //显示交互菜单
    std::cout << "******功能菜单******\n";
    std::cout << "* 1.获取时间       *\n";
    std::cout << "* 2.获取时间(100次)*\n";
    std::cout << "* 3.获取名字       *\n";
    std::cout << "* 4.获取客户端列表 *\n";
    std::cout << "* 5.发送消息       *\n";
    std::cout << "* 6.断开连接       *\n";
    std::cout << "* 7.退出           *\n";
    std::cout << "*请输入序号选择功能*\n";
    //创建监听线程
    int res = pthread_create(&_listenThread, NULL, ListenService, &socketFd);
    if(res != 0){
        throw MySocketException(THREAD_FAILED);
    }
    //创建反馈线程
    res = pthread_create(&_feedbackThread, NULL, FeedbackService, NULL);
    if(res != 0){
        throw MySocketException(THREAD_FAILED);
    }
    //主线程等待用户输入并send
    while(true){
        int input;
        scanf("%d", &input);
        if(!isConnected){
            break;
        }
        Packet packet;
        switch (input)
        {
        case 1:
            packet.loadTimeRequest();
            packet.Send(socketFd);
            break;
        case 2:
            for(int i = 0; i < 100; i++){
                Packet test;
                test.loadTimeRequest();
                test.Send(socketFd);
            }
            break;
        case 3:
            packet.loadNameRequest();
            packet.Send(socketFd);
            break;
        case 4:
            packet.loadClientListRequest();
            packet.Send(socketFd);
            break;
        case 5:
            {
                int target;
                std::cout << "请输入接收方序号: " << std::flush;
                scanf("%d", &target);

                std::cout << "请输入内容(以回车结束)\n";
                string content;
                std::cin >> content; 
                
                packet.loadMessageRequest(target, content + "\n");
                packet.Send(socketFd);
            }
            break;
        case 6:
            shutdown(socketFd, SHUT_RDWR);
            close(socketFd);
            isConnected = false;
            return;
        case 7:
            close(socketFd);
            isConnected = false;
            isQuit = true;
            return;
        default:
            break;
        }
    }
}

Client::~Client(){
}

void* ListenService(void* _pt){
    int socketFd = *(int*)(_pt);

    while(true){
        if(!isConnected) break;
        Packet packet;
        packet.Receive(socketFd);
        if(!isConnected) break;
        if(packet.getPayloadSize() != -1){
            std::vector<string> responses;
            packet.getMessages(responses);
            for(auto response: responses){
                msgQueue.push(response);
            }
        }
        else{
            std::cout << "Server closed\nPlease input any number to disconnect" << std::endl;
            isConnected = false;
            close(socketFd);
        }
    }
    return NULL;
}
void* FeedbackService(void* _pt){
    int test = 1;
    while (true){
        if(!isConnected && !msgQueue.size()){
            break;
        }
        if(msgQueue.size() != 0){
            string thisMsg = msgQueue.front();
            msgQueue.pop();
            if(thisMsg == "Server Disconnect"){
                break;
            }
            string content, sender, receiver;
            switch (ParseResponse(thisMsg, content, sender, receiver))
            {
            case Msg_type::TIME_OK:
                std::cout << test <<"Time: " << content << "\n";
                test++;
                break;
            case Msg_type::NAME_OK:
                std::cout << "Name: " << content << "\n";
                break;
            case Msg_type::LIST_OK:
                std::cout << content << "\n";
                break;
            case Msg_type::POST_OK:
                std::cout << "Post Successfully\n";
                break;
            case Msg_type::POST_UNKNOWN_NUM:
                std::cout << "Post Failed: No such client\n";
                break;
            case Msg_type::INST_MSG:
                std::cout << "Server Instruction:\n" << receiver <<"(Local) has received a message from " << sender << "Content:\n" << content << std::endl;
                break;
            case Msg_type::ERROR:
                std::cout << "Error\n";
                break;
            default:
                break;
            }
        }
    }
    
    return NULL;
}

Msg_type ParseResponse(string& response, string& content, string& sender, string& receiver){    
    string firstLine = response.substr(0, response.find('\n'));

    if(firstLine == "RESP TIME OK"){
        content = response.substr(response.find('\n')+1);
        return Msg_type::TIME_OK;
    }
    else if (firstLine == "RESP NAME OK"){
        content = response.substr(response.find('\n')+1);
        return Msg_type::NAME_OK;
    }
    else if (firstLine == "RESP LIST OK"){
        content = response.substr(response.find('\n')+1);
        return Msg_type::LIST_OK;
    }
    else if (firstLine == "RESP POST OK"){
        return Msg_type::POST_OK;
    }
    else if (firstLine == "RESP POST UNKNOWN NUM"){
        return Msg_type::POST_UNKNOWN_NUM;
    }
    else if (firstLine == "RESP POST UNCLEAN DISCONNECT"){
        return Msg_type::POST_UNCLEAN_DISCONNECT;
    }
    else if (firstLine == "INST MSG"){
        content = response.substr(response.find('\n')+1);
        sender = content.substr(6, content.find('\n')-5);
        content = content.substr(content.find('\n')+1);
        receiver = content.substr(4, content.find('\n')-4);
        content = content.substr(content.find('\n')+1);

        return Msg_type::INST_MSG;
    }
    else{
        return Msg_type::ERROR;
    }
}