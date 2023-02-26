#include "Server.h"
#include <iostream>

int main() {
    try{
        Server server;
        server.Initialize();
        server.Listen(5797);
        server.Start();
    }
    catch(MySocketException e){
        std::cerr << "[Error]: " << e.what() << "\n";
    }
    return 0;
}