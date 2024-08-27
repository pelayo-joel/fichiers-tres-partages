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

int FTP_Socket::sendFile(int socket, const char *fileName)
{
    char buffer_[MAX_SIZE_PACKET];
    std::ifstream file;

    file.open(fileName, std::ios::binary);

    if(!file.is_open())
    {
        std::cerr << "Error: Could not open file " << fileName << std::endl;
        return -1;
    }

    while(!file.eof()) {
        std::cout << "begin while" << std::endl;
        
        
        file.read(buffer_, MAX_SIZE_PACKET);
        std::cout << buffer_ << std::endl;
        int bytesRead = file.gcount();
        std::cout << "bytesRead: " << bytesRead << std::endl;
        ::send(socket, buffer_, bytesRead, 0);
        std::cout << "end while" << std::endl;

    }
    return 0;
}

int FTP_Socket::recvFile(int socket, const char *fileName)
{
    char buffer_[MAX_SIZE_PACKET];
    std::ofstream file;

    file.open(fileName, std::ios::out);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << fileName << std::endl;
        return -1;
    }

    int bytesReceived = read(socket, buffer_, 1024);
    int fileSize = 0;
    std::cout << "bytesReceived: " << bytesReceived << std::endl;
    std::cout << "Receiving file..." << std::endl;
    while (fileSize != bytesReceived)
    {
        // std::cout << "Received: " << buffer_ << std::endl;
        file.write(buffer_, bytesReceived);
        fileSize += bytesReceived;
    }

    std::cout << "File received successfully" << std::endl;

    return 0;
}


int FTP_Socket::get_socketFD() { return socketFD_; }
struct sockaddr_in FTP_Socket::get_sinAddress() { return sinAddress_; }

void FTP_Socket::set_socketFD(int socket) { socketFD_ = socket; }
void FTP_Socket::set_sinAddress(struct sockaddr_in sinAddr) { sinAddress_ = sinAddr; }