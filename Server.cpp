#include "Server.h"
#include "Packet.h"
#include <vector>
#include <iostream>
#include <ctime>


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <exception>
#include <stdexcept>
#include <unistd.h>
#include <netinet/tcp.h>

using std::vector;

vector<client_info> Client_List;

void Server::Listen(int port, string addr){
    _addr = addr;
    _port = port;

    sockaddr_in addrin;
    addrin.sin_family = AF_INET;
    addrin.sin_port = htons(_port);
    addrin.sin_addr.s_addr = addr == "" ? htonl(INADDR_ANY) : inet_addr(_addr.c_str());

    memset(&addrin.sin_zero, 0, sizeof(addrin.sin_zero));

    if(bind(socketFd, (sockaddr*)&addrin, sizeof(addrin)) != 0){
        throw MySocketException(BINDING_FAILED);
    }
    listen(socketFd, 5);
}

void Server::Start(){
    while (true){
        int socklen=sizeof(sockaddr_in);
        sockaddr_in client_addr;
        int clientFd = accept(socketFd, (sockaddr*)&client_addr, (socklen_t*)&socklen);
        if(clientFd != -1){
            //新的客户端连接
            int client_port = ntohs(client_addr.sin_port);
            string client_address = inet_ntoa(client_addr.sin_addr);
            client_info info{clientFd, client_address, client_port};
            Client_List.push_back(info);

            std::cout << "[Log]: Connect to " << client_address << ":" << std::to_string(client_port) << std::endl;

            //创建新的服务线程
            pthread_t serviceThread;
            int res = pthread_create(&serviceThread, NULL, ClientService, (void*)&info);
            if(res != 0){
                throw MySocketException(THREAD_FAILED);
            }
        }
        else{
            throw MySocketException(ACCEPTING_FAILED);
        }
    }  
}


void* ClientService(void* info_pt){
    int clientFd = ((client_info*)info_pt) -> clientFd;
    string client_addr = ((client_info*)info_pt) -> addr;
    int client_port = ((client_info*)info_pt) -> port;
    //循环调用receive
    while(true){
        Packet recvPacket;
        recvPacket.Receive(clientFd);
        if(recvPacket.getPayloadSize() != -1){
            vector<string> requests;
            recvPacket.getMessages(requests);
            for(auto request: requests){
                int targetNum;
                string content;
                Packet sendPacket;
                switch (ParseRequest(request, targetNum, content))
                {
                case Msg_type::REQU_TIME:
                    {
                        time_t now;
                        time(&now);
                        tm* t = localtime(&now);

                        char temp[30]; //格式化输出
                        sprintf(temp, "%d-%02d-%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec); //将时间格式化输出
                        sendPacket.loadTimeResponse(temp);
                        sendPacket.Send(clientFd);
                    }
                    break;
                case Msg_type::REQU_NAME:
                    {
                        char buffer[256]={0};
                        gethostname(buffer,256);
                        string hostName = string(buffer) + "\n";
                        sendPacket.loadNameResponse(hostName);
                        sendPacket.Send(clientFd);
                    }
                    break;
                case Msg_type::REQU_LIST:
                    {
                        string listStr = "";

                        int sz = Client_List.size();

                        for(int i = 0; i < sz; i++){
                            string client = "No." + std::to_string(i) + " " + Client_List[i].addr + ":" + std::to_string(Client_List[i].port);
                            if(Client_List[i].clientFd == clientFd){
                                client += " (You are here)";
                            }
                            listStr += client + "\n";
                        }

                        sendPacket.loadClientListResponse(listStr);
                        sendPacket.Send(clientFd);
                    }
                    break;
                case Msg_type::REQU_POST:
                    if(targetNum >= 0 && targetNum < Client_List.size()){
                        int srcNum = -1;

                        for(int i = 0; i < Client_List.size(); i++){
                            if(clientFd == Client_List[i].clientFd){
                                srcNum = i;
                                break;
                            }
                        }

                        Packet postBPacket;
                        postBPacket.loadMessageInstruction(srcNum, targetNum, content + "\n");
                        postBPacket.Send(Client_List[targetNum].clientFd);


                        Packet postAPacket;
                        postAPacket.loadMessageResponse(1); //成功
                        postAPacket.Send(clientFd);
                    }
                    else{
                        Packet postAPacket;
                        postAPacket.loadMessageResponse(0);
                        postAPacket.Send(clientFd);
                    }    
                    break;
                case Msg_type::ERROR:
                    break;
                default:
                    break;
                }
            }
        }
        else{
            //这个客户端断开连接
            std::cout << "[Log]: Disconnect " << client_addr << ":" << std::to_string(client_port) << std::endl;

            close(clientFd);
            for(auto iter = Client_List.begin(); iter != Client_List.end();){
                if(iter->clientFd == clientFd){
                    iter = Client_List.erase(iter);
                }
                else{
                    ++iter;
                }
            }
            break;
        }
        
    }

    return NULL;
}

Msg_type ParseRequest(string& request, int& targetClientNum, string& msg){
    string firstLine = request.substr(0, request.find('\n'));


    if(firstLine == "REQU TIME"){
        return Msg_type::REQU_TIME;
    }
    else if(firstLine == "REQU NAME"){
        return Msg_type::REQU_NAME;
    }
    else if(firstLine == "REQU LIST"){
        return Msg_type::REQU_LIST;
    }
    else if(firstLine == "REQU POST"){
        msg = request.substr(request.find('\n')+1);
        targetClientNum = atoi(msg.substr(4, request.find('\n')-4).c_str());
        msg = msg.substr(msg.find('\n')+1);
        return Msg_type::REQU_POST;
    }
    else{
        return Msg_type::ERROR;
    }
}
Server::~Server()
{
}