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

#define PACKET_SIZE sizeof(FTP_Packet)
#define MAX_SIZE_USER 64
#define MAX_SIZE_MESSAGE 128
#define MAX_SIZE_BUFFER 512

#define BUFFER_CHUNK_SIZE 1024
#define CHUNK_SIZE 16 * 1024

enum command
{
    UPLOAD,
    DOWNLOAD,
    DELETE,
    LIST,
    CREATE,
    RM,
    RENAME
};


class FTP_Packet
{
private:
    // Header
    int64_t fileSize;
    char fileName[MAX_SIZE_MESSAGE];
    char userName[MAX_SIZE_USER];
    char folderName[MAX_SIZE_MESSAGE];
    char path[MAX_SIZE_MESSAGE];
    command command_;
  
    // Data
    // char rawData[MAX_SIZE_PACKET];
    
public:
    FTP_Packet();
    FTP_Packet(const FTP_Packet &base);
    virtual ~FTP_Packet();
    FTP_Packet &operator=(const FTP_Packet &other);

    char* get_FileName();
    int64_t get_FileSize();
    char* get_Username();
    char* get_FolderName();
    char* get_Path();
    // char* get_RawData();
    command get_Command();


    void set_FileName(char* newName);
    void set_FileSize(const int64_t newSize);
    void set_Username(char* newName);
    void set_FolderName(char* newName);
    void set_Path(char* newPath);
    // void set_RawData(void* data);
    void set_Command(command newCommand);
};