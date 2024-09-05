#pragma once

#include <pthread.h>
#include <thread>
#include <queue>

#include "FTP_Socket.hpp"

#define DESTINATION_PATH "data/"

class Server : public FTP_Socket
{
private:
    int addrlen_;
    // std::queue<std::thread> threads_;

    int bind();
    int listen(int maxQueue);

public:
    Server(int port, int maxQueue);
    Server(const Server &base);
    ~Server();
    Server &operator=(const Server &base);

    int accept();
    int createClientThread(int clientFD);
    // void parsePacket(int clientFD);
    char* createUserFolder(char* destPath, char* username);
    int recvClientUpload(FTP_Packet packet);
    int deleteFile(char* fileName, char* username);
    int checkClientAuthentication(int client, char* username, char* password);
    int createNewUser(char* username, char* password, char* authPath);

    int getAddrlen();
    // std::queue<std::thread> getClients();

    void setAddrlen(int addrlen);
    // void setClientsThread(std::queue<std::thread> clientsThread);
};