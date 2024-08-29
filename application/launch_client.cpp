#include "../classes/include/client.hpp"
#include <iostream>



int main(int argc, char *argv[])
{
    if (argc == 0 && argc > 4)
    {
        std::cerr << "Usage : " << argv[0] << " <port>" << std::endl;
        return -1;
    }
    int port = std::stoi(argv[1]);
    char* filePath = argv[2];

    std::cout << "File: " << filePath << std::endl;

    Client client = Client(port);
    int clientSocket = client.get_socketFD();

    client.sendFile(clientSocket, filePath);
    
    char response[2048];
    client.recv(clientSocket, response, 0);

    // char message[1024] = "ping";
    // client.send(clientSocket, message, 0);
    // std::cout << "Message sent: " << message << std::endl;

    // client.recv(clientSocket, response, 0);
    // std::cout << "Server response: " << response << std::endl;

    return 0;
}