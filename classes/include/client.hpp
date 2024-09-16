
#pragma once

#include "FTP_Socket.hpp"

#define DEFAULT_PATH "clientFiles/"


class Client : public FTP_Socket
{
private:
    int connect();

public:
    Client(char* serverAddress, int port);
    Client(const Client &base);
    ~Client();
    Client &operator=(const Client &base);

    void userRecognition(char* username);
    int userAuthentication(char* username);
    int recvServerDownload();
    int recvServerList();
};
