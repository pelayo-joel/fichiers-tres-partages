#include "../include/server.hpp"

Server::Server(int port, int maxQueue) : FTP_Socket(port)
{
    int serverSocket = this->get_socketFD();
    // int enableReuse = 1;

    sinAddress_.sin_addr.s_addr = INADDR_ANY;
    char serverIP[INET_ADDRSTRLEN] = "127.0.0.1";
    inet_ntop(AF_INET, &sinAddress_.sin_addr, serverIP, INET_ADDRSTRLEN);

    // Allows the server socket to connect to multiple client sockets
    // setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&enableReuse, sizeof(enableReuse));

    bind();
    listen(maxQueue);
    std::cout << "Servers listens on port " << port << std::endl;
}

Server::~Server() {}

Server::Server(const Server &base) : FTP_Socket(base) {}

Server &Server::operator=(const Server &base)
{
    if (this != &base)
    {
        FTP_Socket::operator=(base);
        masterFD_ = base.masterFD_;
        activity_ = base.activity_;
        addrlen_ = base.addrlen_;
        maxClients_ = base.maxClients_;
        // clients_ = base.clients_;
        readfds_ = base.readfds_;
    }
    return *this;
}


int Server::bind()
{
    int serverSocket = this->get_socketFD();
    return ::bind(serverSocket, (struct sockaddr *)&sinAddress_, sizeof(sinAddress_));
}

int Server::listen(int maxQueue)
{
    int serverSocket = this->get_socketFD();
    return ::listen(serverSocket, maxQueue);
}

int Server::recvClientUpload(int socketFD, char* destPath)
{
    char buffer_[MAX_SIZE_PACKET];
    char filePath[256];
    std::ofstream file;
    

    std::memcpy(buffer_, FTP_Socket::recvFile(socketFD), MAX_SIZE_PACKET);
    FTP_Packet newPacket = *(FTP_Packet*) buffer_;

    char* destinationPath = createDestinationFolder(destPath);
    char* userPath = createUserFolder(destinationPath, newPacket.get_Username());
    strcpy(filePath, pathToReceivedFile(userPath, newPacket.get_FileName()));

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

    return 0;
}

char* Server::createUserFolder(char* destPath, char* username) 
{
    std::string mainFolder = destPath;
    std::string userFolder = mainFolder + username;
    if (!std::filesystem::exists(userFolder))
    {
        std::filesystem::create_directory(userFolder);
    }

    auto strLen = userFolder.length();
    char* userFolderPath = new char[strLen + 2];
    std::memcpy(userFolderPath, userFolder.c_str(), strLen);

    userFolderPath[strLen] = '/';
    userFolderPath[strLen+1] = '\0';
    return userFolderPath;
}


int Server::recvClientUpload(int socketFD, char* destPath)
{
    char buffer_[MAX_SIZE_PACKET];
    char filePath[256];
    std::ofstream file;
    

    std::memcpy(buffer_, FTP_Socket::recvFile(socketFD), MAX_SIZE_PACKET);
    FTP_Packet newPacket = *(FTP_Packet*) buffer_;

    char* destinationPath = createDestinationFolder(destPath);
    char* userPath = createUserFolder(destinationPath, newPacket.get_Username());
    strcpy(filePath, pathToReceivedFile(userPath, newPacket.get_FileName()));

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

    return 0;
}

char* Server::createUserFolder(char* destPath, char* username) 
{
    std::string mainFolder = destPath;
    std::string userFolder = mainFolder + username;
    if (!std::filesystem::exists(userFolder))
    {
        std::filesystem::create_directory(userFolder);
    }

    auto strLen = userFolder.length();
    char* userFolderPath = new char[strLen + 2];
    std::memcpy(userFolderPath, userFolder.c_str(), strLen);

    userFolderPath[strLen] = '/';
    userFolderPath[strLen+1] = '\0';
    return userFolderPath;
}

int Server::accept()
{
    int serverSocket = this->get_socketFD();
    int addrlen = sizeof(sinAddress_);
    return ::accept(serverSocket, (struct sockaddr *)&sinAddress_, (socklen_t *)&addrlen);
}
int Server::getMasterFD() { return masterFD_; }
int Server::getClientFD() { return clientFD_; }
int Server::getActivity() { return activity_; }
int Server::getAddrlen() { return addrlen_; }
int Server::getMaxClients() { return maxClients_; }
// std::vector<int> Server::getClients() { return clients_; }
fd_set Server::getReadfds() { return readfds_; }

void Server::setMasterFD(int masterFD) { masterFD_ = masterFD;}
void Server::setClientFD(int clientFD) { clientFD_ = clientFD;}
void Server::setActivity(int activity) { activity_ = activity; }
void Server::setAddrlen(int addrlen) { addrlen_ = addrlen; }
void Server::setMaxClients(int maxClients) { maxClients_ = maxClients; }
// void Server::setClients(std::vector<int> clients) { clients_ = clients; }
void Server::setReadfds(fd_set readfds) { readfds_ = readfds; }