
#pragma once

#include "FTP_Socket.hpp"

#define DESTINATION_PATH "clientFiles/"


class Client : public FTP_Socket
{
private:
    int connect();

public:
    Client(char* serverAddress, int port);
    Client(const Client &base);
    ~Client();
    Client &operator=(const Client &base);

    int recvServerDownload();
    int deleteFileOnServer(FTP_Packet packet);
};
