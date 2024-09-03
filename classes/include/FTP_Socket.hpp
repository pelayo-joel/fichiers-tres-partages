#pragma once

#include <iostream>
#include <fstream>
#include <cstdio>  
#include <cstring>
#include <limits.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <filesystem>

#include "FTP_Packet.hpp"

class FTP_Socket
{
private:
    int socketFD_;

protected:
    struct sockaddr_in sinAddress_;
    // char* pathToReceivedFile(char* fileName);
    char* createDestinationFolder(char* username, char* destPath);
    char* recvFile(int clientSocket);

public:
    FTP_Socket(int port);
    FTP_Socket(const FTP_Socket &base);
    virtual ~FTP_Socket();
    FTP_Socket &operator=(const FTP_Socket &other);

    ssize_t recv(int clientSocket, void* buffer, int flag);
    ssize_t send(int socket, void* buffer, int flag);
    int sendFile(int clientSocket, char* filePath, char* username);

    char* pathToReceivedFile(char* folderPath, char* fileName);
    char* createDestinationFolder(char* destPath);
    // int deleteFile(char* fileName, char* username);

    int get_socketFD();
    struct sockaddr_in get_sinAddress();

    void set_socketFD(int socket);
    void set_sinAddress(struct sockaddr_in sinAddr);
};