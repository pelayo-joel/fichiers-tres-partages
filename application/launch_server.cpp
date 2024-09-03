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

    while (true)
    {
        if ((newClientFD = server.accept()) > 0) {
            // server.recvClientUpload(newClientFD, "data/");
            server.createClientThread(newClientFD);
            // char response[1024] = "File received by server";
            // server.send(newClientFD, response, 0);
        }
    }
    

    return 0;
}