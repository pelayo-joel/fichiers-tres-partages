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
    // char buffer_[MAX_SIZE_PACKET];
    FTP_Packet newPacket = FTP_Packet();
    char* fileName = newPacket.pathParsing(filePath);
    std::ifstream file; 

    file.open(filePath, std::ios::binary);

    if(!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return -1;
    }

    while(!file.eof()) {
        std::cout << "begin while" << std::endl;
        file.read(newPacket.get_RawData(), MAX_SIZE_PACKET);
        int bytesRead = file.gcount();
        newPacket.set_FileName(fileName);
        newPacket.set_FileSize(bytesRead);
        ::send(socket, &newPacket, bytesRead, 0);
    }

    std::cout << "File successfully sent !" << std::endl;

    return 0;
}

int FTP_Socket::recvFile(int socket)
{
    char buffer_[MAX_SIZE_PACKET];
    std::ofstream file;

    int bytesReceived = read(socket, buffer_, MAX_SIZE_PACKET);
    FTP_Packet newPacket = *(FTP_Packet*) buffer_;

    // if (!file.is_open())
    // {
    //     std::cerr << "Error: Could not open file " << fileName << std::endl;
    //     return -1;
    // }

    file.open(newPacket.get_FileName(), std::ios::out);
    // std::cout << "bytesReceived: " << bytesReceived << std::endl;
    // std::cout << "Receiving file..." << std::endl;
    file.write(newPacket.get_RawData(), bytesReceived);
    // fileSize += bytesReceived;
    // while (fileSize <= bytesReceived)
    // {
    //     // std::cout << "Received: " << buffer_ << std::endl;
    // }

    std::cout << "File received successfully" << std::endl;

    return 0;
}


int FTP_Socket::get_socketFD() { return socketFD_; }
struct sockaddr_in FTP_Socket::get_sinAddress() { return sinAddress_; }

void FTP_Socket::set_socketFD(int socket) { socketFD_ = socket; }
void FTP_Socket::set_sinAddress(struct sockaddr_in sinAddr) { sinAddress_ = sinAddr; }