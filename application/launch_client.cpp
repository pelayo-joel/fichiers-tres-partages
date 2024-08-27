#include "../classes/include/client.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc == 0)
    {
        std::cerr << "Usage : " << argv[0] << " <port>" << std::endl;
        return -1;
    }

    int port = std::stoi(argv[1]);

    Client client = Client(port);
    int clientSocket = client.get_socketFD();

    char message[1024] = "ping";
    client.send(clientSocket, message, 0);
    std::cout << "Message sent: " << message << std::endl;

    char response[2048];
    client.recv(clientSocket, response, 0);
    std::cout << "Server response: " << response << std::endl;

    return 0;
}