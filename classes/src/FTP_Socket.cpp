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

ssize_t FTP_Socket::recv(int clientSocket, void *buffer)
{
    return ::recv(clientSocket, buffer, PACKET_SIZE, 0);
}

ssize_t FTP_Socket::send(int socket, void *buffer)
{
    return ::send(socket, buffer, PACKET_SIZE, 0);
}

int FTP_Socket::sendFile(int socket, char* filePath, const int64_t fileSize)
{
    if (!std::filesystem::exists(filePath) && std::filesystem::is_directory(filePath))
    {
        return -3;
    }
    
    std::ifstream file(filePath, std::ifstream::binary);
    if(!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return -1;
    }

    char* buffer = new char[CHUNK_SIZE];
    int64_t bytesLeft = fileSize;
    while(bytesLeft != 0) 
    {
        const int64_t ssize = std::min(bytesLeft, (int64_t) CHUNK_SIZE);

        if (!file.read(buffer, ssize)) 
        { 
            return -2; 
        }
        const int l = sendFileBuffer(socket, buffer, (int) ssize);
        bytesLeft -= l;
    }
    
    delete[] buffer;

    file.close();
    std::cout << "File successfully sent !" << std::endl;

    return 0;
}

int FTP_Socket::recvFile(int socket, char* filePath, const int64_t fileSize) {
    // if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath))
    // {
    //     return -3;
    // }
    std::ofstream file(filePath, std::ofstream::binary);
    if(!file.is_open())
    {
        std::cerr << "Error: Could not create or open file " << filePath << std::endl;
        return -1;
    }
    
    char* buffer = new char[CHUNK_SIZE];
    int64_t bytesLeft = fileSize;
    while (bytesLeft != 0)
    {
        const int r = recvFileBuffer(socket, buffer, (int) std::min(bytesLeft, (int64_t) CHUNK_SIZE));
        if ((r < 0) || !file.write(buffer, r)) 
        { 
            return -2; 
        }
        bytesLeft -= r;
    }

    delete[] buffer;
    file.close();
    return 0;
}



int FTP_Socket::recvFileBuffer(int socket, char* buffer, int bufferSize) {
    int i = 0;

    while (i < bufferSize)
    {
        const int l = ::recv(socket, &buffer[i], std::min(BUFFER_CHUNK_SIZE, bufferSize - i), 0);
        if (l < 0) 
        { 
            return l; 
        }
        i += l;
    }
    return i;
}

int FTP_Socket::sendFileBuffer(int socket, char* buffer, int bufferSize) {
    int i = 0;

    while (i < bufferSize)
    {
        const int l = ::send(socket, &buffer[i], std::min(BUFFER_CHUNK_SIZE, bufferSize - i), 0);
        if (l < 0) 
        { 
            return l; 
        }
        i += l;
    }
    return i;
}

char* FTP_Socket::pathToReceivedFile(char* folderPath, char* fileName) {
    std::string strFilename = fileName;
    strFilename.insert(0, folderPath);

    auto strLen = strFilename.length();

    char* filePath = new char[strLen + 2];
    std::memcpy(filePath, strFilename.c_str(), strLen);

    filePath[strLen] = '\0';
    return filePath;
}

int64_t FTP_Socket::getFileSize(char* fileName) {
    std::ifstream in_file(fileName, std::ios::binary);
    in_file.seekg(0, std::ios::end);
    int64_t len = in_file.tellg();
    return len;
}

int FTP_Socket::get_socketFD() { return socketFD_; }
struct sockaddr_in FTP_Socket::get_sinAddress() { return sinAddress_; }

void FTP_Socket::set_socketFD(int socket) { socketFD_ = socket; }
void FTP_Socket::set_sinAddress(struct sockaddr_in sinAddr) { sinAddress_ = sinAddr; }