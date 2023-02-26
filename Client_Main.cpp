#include "Client.h"
#include <iostream>

bool isQuit = false;

void PrintPrompt();
int main(){
    while (true){
        PrintPrompt();
        int input;
        scanf("%d", &input);
        if(input == 2){
            std::cout << "Bye!\n";
            break;
        }
        string addr;
        int port;
        std::cout << "请输入服务器地址(47.102.200.110): " << std::flush;
        std::cin >> addr;
        std::cout << "请输入服务器端口(5797): " << std::flush;
        std::cin >> port;

        try{
            Client client;
            client.Initialize();
            client.Connect(addr, port);
            client.Start();
            if(isQuit){
                std::cout << "Bye!\n";
                return 0;
            }
        }
        catch(MySocketException e){
            std::cerr << e.what() << "\n";
        }
    }
    

    return 0;
}
void PrintPrompt(){
    std::cout << "******功能菜单******\n";
    std::cout << "* 1.连接           *\n";
    std::cout << "* 2.退出           *\n";
    std::cout << "*请输入序号选择功能*\n";
}