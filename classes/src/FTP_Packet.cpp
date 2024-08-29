#include "../include/FTP_Packet.hpp"

FTP_Packet::FTP_Packet() {

}

FTP_Packet::FTP_Packet(const FTP_Packet &base) {
    fileSize = base.fileSize;
    memcpy(fileName, base.fileName, MAX_SIZE_PACKET);
    memcpy(rawData, base.rawData, MAX_SIZE_PACKET);
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
ssize_t FTP_Packet::get_FileSize() { return fileSize; }
char* FTP_Packet::get_RawData() { return rawData; }

void FTP_Packet::set_FileName(char* newName) {
    std::string strPath = newName;
    std::string parsedFilename = strPath.substr(strPath.find_last_of("/\\") + 1);
    // std::cout << "Return pathParsing : " << &parsedFilename[0] << std::endl;
    strcpy(fileName, &parsedFilename[0]);
}
void FTP_Packet::set_FileSize(ssize_t newSize) {
    fileSize = newSize;
}
void FTP_Packet::set_RawData(void* data) {
    memcpy(rawData, data, MAX_SIZE_PACKET);
}

// char* FTP_Packet::pathParsing(char* filePath) {
//     return &fileName[0];
// }
