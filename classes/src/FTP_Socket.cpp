#include "../include/FTP_Socket.hpp"


FTP_Socket::FTP_Socket(int port)
{
    socketFD_ = socket(AF_INET, SOCK_STREAM, 0);

    sinAddress_.sin_family = AF_INET;
    sinAddress_.sin_port = htons(port);
}

FTP_Socket::FTP_Socket(const FTP_Socket &base)
{
    socketFD_ = base.socketFD_;
    sinAddress_ = base.sinAddress_;
}

FTP_Socket::~FTP_Socket()
{
    ::close(socketFD_);
}

FTP_Socket &FTP_Socket::operator=(const FTP_Socket &other)
{
    if (this != &other)
    {
        socketFD_ = other.socketFD_;
        sinAddress_ = other.sinAddress_;
    }

    return *this;
}

ssize_t FTP_Socket::recv(int clientSocket, void *buffer, int flag)
{
    return ::recv(clientSocket, buffer, 2048, flag);
}

ssize_t FTP_Socket::send(int socket, void *buffer, int flag)
{
    return ::send(socket, buffer, 2048, flag);
}

int FTP_Socket::sendFile(int socket, char* filePath)
{
    FTP_Packet newPacket = FTP_Packet();

    std::ifstream file; 

    file.open(filePath, std::ios::binary);

    if(!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return -1;
    }

    while(!file.eof()) {
        file.read(newPacket.get_RawData(), MAX_SIZE_PACKET);
        int bytesRead = file.gcount();

        newPacket.set_FileName(filePath);
        newPacket.set_FileSize(bytesRead);
        ::send(socket, &newPacket, sizeof(FTP_Packet), 0);
    }

    file.close();
    std::cout << "File successfully sent !" << std::endl;

    return 0;
}

int FTP_Socket::recvFile(int socket)
{
    char buffer_[MAX_SIZE_PACKET];
    char filePath[128];
    std::ofstream file;

    int bytesReceived = read(socket, buffer_, MAX_SIZE_PACKET);

    FTP_Packet newPacket = *(FTP_Packet*) buffer_;
    strcpy(filePath, pathToReceivedFile(newPacket.get_FileName()));

    file.open(filePath, std::ios::out);
    file.write(newPacket.get_RawData(), newPacket.get_FileSize());

    // while ((bytesReceived = read(socket, buffer_, MAX_SIZE_PACKET)) > 0)
    // {
    //     FTP_Packet newPacket = *(FTP_Packet*) buffer_;
    //     strcpy(filePath, pathToReceivedFile(newPacket.get_FileName()));

    //     file.open(filePath, std::ios::out);
    //     file.write(newPacket.get_RawData(), newPacket.get_FileSize());
    // }

    file.close();
    std::cout << "File received successfully at: " << filePath << std::endl;

    return 0;
}

char* FTP_Socket::pathToReceivedFile(char* fileName) {
    std::string strFilename = fileName;
    strFilename.insert(0, "serverFiles/");

    auto strLen = strFilename.length();

    char* filePath = new char[strLen + 2];
    std::memcpy(filePath, strFilename.c_str(), strLen);

    filePath[strLen] = '\0';
    return filePath;
}

int FTP_Socket::get_socketFD() { return socketFD_; }
struct sockaddr_in FTP_Socket::get_sinAddress() { return sinAddress_; }

void FTP_Socket::set_socketFD(int socket) { socketFD_ = socket; }
void FTP_Socket::set_sinAddress(struct sockaddr_in sinAddr) { sinAddress_ = sinAddr; }