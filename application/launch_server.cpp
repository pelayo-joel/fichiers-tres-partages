#include "../classes/include/server.hpp"

int main()
{

    AppLogs& logger = AppLogs::Instance();
    logger.currentLevel = 9;
    logger.path = "./logs/";
    logger.fileName = "logs_server";

    const int maxQueue = 5;

    Server server = Server(maxQueue);
    int newClientFD;

    signal(SIGINT, Server::handleLeave);

    while (true)
    {
        if ((newClientFD = server.accept()) > 0) {
            server.createClientThread(newClientFD);

        }
    }

    return 0;
}