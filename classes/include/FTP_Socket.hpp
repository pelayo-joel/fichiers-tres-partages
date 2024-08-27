#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#define MAX_SIZE_PACKET 4096
#define IP "127.0.0.1"

class FTP_Socket
{
private:
    int socketFD_;

protected:
    struct sockaddr_in sinAddress_;

public:
    FTP_Socket(int port);
    FTP_Socket(const FTP_Socket &base);
    virtual ~FTP_Socket();
    FTP_Socket &operator=(const FTP_Socket &other);

    ssize_t recv(int clientSocket, void *buffer, int flag);
    ssize_t send(int socket, void *buffer, int flag);
    int sendFile(int clientSocket, const char *fileName);
    int recvFile(int clientSocket, const char *fileName);

    int get_socketFD();
    struct sockaddr_in get_sinAddress();

    void set_socketFD(int socket);
    void set_sinAddress(struct sockaddr_in sinAddr);
};