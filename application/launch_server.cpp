#include "../classes/include/server.hpp"

int main(int argc, char *argv[])
{
    if (argc == 0)
    {
        std::cerr << "Usage : " << argv[0] << " <port>" << std::endl;
        return -1;
    }

    int port = std::stoi(argv[1]);
    const int maxQueue = 5;

    Server server = Server(port, maxQueue);
    int newClientFD;

    // char serverMsg[2048] = "pong";
    // server.send(clientFD, serverMsg, 0);
    // std::cout << "Sent: " << serverMsg << std::endl;

    while (true)
    {
        if ((newClientFD = server.accept()) > 0) {
            server.recvClientUpload(newClientFD, "data/");
            char response[1024] = "File received by server";
            server.send(newClientFD, response, 0);
        }
    }
    

    return 0;
}