#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <limits.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cstdint>

#define MAX_SIZE_PACKET 65536


class FTP_Packet
{
private:
    // Header
    ssize_t fileSize;
    char fileName[128];

    // Data
    char rawData[MAX_SIZE_PACKET];
    
public:
    FTP_Packet();
    FTP_Packet(const FTP_Packet &base);
    virtual ~FTP_Packet();
    FTP_Packet &operator=(const FTP_Packet &other);

    char* get_FileName();
    ssize_t get_FileSize();
    char* get_RawData();

    void set_FileName(char* newName);
    void set_FileSize(ssize_t newSize);
    void set_RawData(void* data);

    char* pathParsing(char* filePath);
};