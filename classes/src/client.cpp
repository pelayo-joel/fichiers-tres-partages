#include "../include/client.hpp"

Client::Client(char* serverAddress, int port) : FTP_Socket(port)
{
    char serverIP[INET_ADDRSTRLEN];
    strcpy(serverIP, serverAddress);
    sinAddress_.sin_addr.s_addr = inet_addr(serverIP);

    ::inet_ntop(AF_INET, &sinAddress_.sin_addr, serverIP, INET_ADDRSTRLEN);
    connect();
}

Client::Client(const Client &base) : FTP_Socket(base) {}

Client::~Client() {}

Client &Client::operator=(const Client &base)
{
    if (this != &base)
    {
        FTP_Socket::operator=(base);
    }
    return *this;
}


int Client::connect()
{
    int clientSocket = this->get_socketFD();
    return ::connect(clientSocket, (struct sockaddr *)&sinAddress_, sizeof(sinAddress_));
}