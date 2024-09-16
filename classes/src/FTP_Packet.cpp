#include "../include/FTP_Packet.hpp"

FTP_Packet::FTP_Packet() { }

FTP_Packet::FTP_Packet(const FTP_Packet &base) {
    fileSize = base.fileSize;
    memcpy(fileName, base.fileName, PACKET_SIZE);
}

FTP_Packet::~FTP_Packet() {}

FTP_Packet &FTP_Packet::operator=(const FTP_Packet &other) 
{
    if (this != &other)
    {
        fileSize = other.fileSize;
    }
    
    return *this;
}

char* FTP_Packet::get_FileName() { return fileName; }
int64_t FTP_Packet::get_FileSize() { return fileSize; }
char* FTP_Packet::get_Username() { return userName; }
char* FTP_Packet::get_FolderName() { return folderName; }
char* FTP_Packet::get_Path() { return path; }
char* FTP_Packet::get_Message() { return message; }
command FTP_Packet::get_Command() { return command_; }


void FTP_Packet::set_FileName(char* newName) {
    std::string strPath = newName;
    std::string parsedFilename = strPath.substr(strPath.find_last_of("/\\") + 1);
    strcpy(fileName, &parsedFilename[0]);
}
void FTP_Packet::set_FileSize(const int64_t newSize) {
    fileSize = newSize;
}
void FTP_Packet::set_Username(char* newName) {
    strcpy(userName, newName);
}
void FTP_Packet::set_FolderName(char* newName) {
    strcpy(folderName, newName);
}
void FTP_Packet::set_Path(char* newPath) {
    strcpy(path, newPath);
}
void FTP_Packet::set_Message(char* newMessage) {
    strcpy(message, newMessage);
}
void FTP_Packet::set_Command(command newCommand) {
    command_ = newCommand;

}
