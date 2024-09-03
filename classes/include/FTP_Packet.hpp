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


enum commands
{
    UPLOAD,
    DOWNLOAD,
    DELETE
};


class FTP_Packet
{
private:
    // Header
    ssize_t fileSize;
    char fileName[128];
    char userName[64];
    commands command;
    // char command[32];
    
    // Data
    char rawData[MAX_SIZE_PACKET];
    
public:
    FTP_Packet();
    FTP_Packet(const FTP_Packet &base);
    virtual ~FTP_Packet();
    FTP_Packet &operator=(const FTP_Packet &other);

    char* get_FileName();
    ssize_t get_FileSize();
    char* get_Username();
    char* get_RawData();
    commands get_Command();

    void set_FileName(char* newName);
    void set_FileSize(ssize_t newSize);
    void set_Username(char* newName);
    void set_RawData(void* data);
    void set_Command(commands newCommand);

    // char* pathParsing(char* filePath);
};