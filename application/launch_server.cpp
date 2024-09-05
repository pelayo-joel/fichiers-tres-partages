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
            server.createClientThread(newClientFD);
        }
    }
    
    // while (server.getClients().size() > 0)
    // {
    //     pthread_t clientThread = server.getClients().front();
    //     server.getClients().pop();
    //     pthread_join(clientThread, NULL);
    // }
    

    return 0;
}