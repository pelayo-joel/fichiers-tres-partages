#pragma once

#include <thread>
#include <queue>
#include <csignal>

#include "FTP_Socket.hpp"
#include "logger.hpp"

#define IP_ADDRESS "127.0.0.1"
#define PORT 3000
// #define DESTINATION_PATH "data/"s

class Server : public FTP_Socket
{
private:
    AppLogs& logger = AppLogs::Instance();

    int bind();
    int listen(int maxQueue);

public:
    Server(int maxQueue);
    Server(const Server &base);
    ~Server();
    Server &operator=(const Server &base);

    int accept();
    int createClientThread(int clientFD);
    char* createFolder(char* username, const char* foldername, const char* path);
    int recvClientUpload(FTP_Packet packet);
    int deleteFile(char* fileName, char* username);
    void displayList(int client, char* username, const char* path);
    void deleteFolder(int client, char* username, const char* path);
    int checkUserExists(char* username);
    int checkClientAuthentication(int client, char* username, char* password);
    int createNewUser(char* username, char* password);
    void static handleLeave(int signal);
};