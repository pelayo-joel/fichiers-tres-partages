#include "../classes/include/server.hpp"

int main()
{
    const int maxQueue = 5;

    Server server = Server(maxQueue);
    int newClientFD;

    while (true)
    {
        if ((newClientFD = server.accept()) > 0) {
            server.createClientThread(newClientFD);
        }
    }
    return 0;
}