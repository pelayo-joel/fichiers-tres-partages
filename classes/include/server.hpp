#pragma once

#include <thread>
#include <queue>
#include <csignal>

#include "FTP_Socket.hpp"
#include "logger.hpp"

#define IP_ADDRESS "127.0.0.1"
#define PORT 3000
// #define DESTINATION_PATH "data/"

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
    int checkUserExists(char* username);
    int checkClientAuthentication(int client, char* username, char* password);
    int waitingUserAuthentication(int clientFD, char* username, char* password);
    
    char* createFolder(char* username, const char* foldername, const char* path);
    int recvClientUpload(int clientFD, FTP_Packet packet);
    int sendClientDownload(int clientFD, FTP_Packet packet);
    int displayList(int clientFD, FTP_Packet packet);
    int deleteFile(int clientFD, char* fileName, char* username);
    int deleteFolder(int clientFD, char* username, const char* path);
    int renameFolder(int clientFD, char* username, const char* oldPath, const char* newFolderName);
    int createNewUser(char* username, char* password);
    void static handleLeave(int signal);
    void userRecognition(int clientFD, char* username); 
    void errorLog(int clientFD, char* username, const char* message, const char* variableMessage = "");
};