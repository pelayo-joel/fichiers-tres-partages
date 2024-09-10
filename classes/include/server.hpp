#pragma once

#include <pthread.h>
#include <thread>
#include <queue>

#include "FTP_Socket.hpp"

#define IP_ADDRESS "127.0.0.1"
#define PORT 3000
#define DESTINATION_PATH "data/"

class Server : public FTP_Socket
{
private:
    // int addrlen_;

    int bind();
    int listen(int maxQueue);

public:
    Server(int maxQueue);
    Server(const Server &base);
    ~Server();
    Server &operator=(const Server &base);

    int accept();
    int createClientThread(int clientFD);
    char* createUserFolder(char* username);
    int recvClientUpload(FTP_Packet packet);
    int deleteFile(char* fileName, char* username);
    int checkUserExists(char* username);
    int checkClientAuthentication(int client, char* username, char* password);
    int createNewUser(char* username, char* password);

    // int getAddrlen();
    // void setAddrlen(int addrlen);
};