
#pragma once

#include "FTP_Socket.hpp"

class Client : public FTP_Socket
{
private:
    int connect();

public:
    Client(int port);
    Client(const Client &base);
    ~Client();
    Client &operator=(const Client &base);
};
