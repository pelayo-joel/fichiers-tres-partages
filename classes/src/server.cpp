#include "../include/server.hpp"

Server::Server(int port, int maxQueue) : FTP_Socket(port)
{
    int serverSocket = this->get_socketFD();

    sinAddress_.sin_addr.s_addr = INADDR_ANY;
    char serverIP[INET_ADDRSTRLEN] = "127.0.0.1";
    inet_ntop(AF_INET, &sinAddress_.sin_addr, serverIP, INET_ADDRSTRLEN);

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


int Server::recvClientUpload(FTP_Packet packet)
{
    char filePath[256];
    std::ofstream file;

    char* destinationPath = createDestinationFolder(DESTINATION_PATH);
    char* userPath = createUserFolder(destinationPath, packet.get_Username());
    strcpy(filePath, pathToReceivedFile(userPath, packet.get_FileName()));

    file.open(filePath, std::ios::out);
    file.write(packet.get_RawData(), packet.get_FileSize());

    file.close();

    return 0;
}

int Server::deleteFile(char* fileName, char* username) 
{
    char completePath[256] = DESTINATION_PATH;
    snprintf(completePath, sizeof(completePath), "data/%s/%s", username, fileName);

    std::cout << "Deleting file complete path: " << completePath << std::endl;

    int status = remove(completePath);
    return status;
}

int Server::createClientThread(int clientFD)
{
    auto packetParsing = [this](int client) {
        char buffer[MAX_SIZE_PACKET];
        char response[2048];
        recv(client, buffer, 0);

        FTP_Packet newPacket = *(FTP_Packet*) buffer;

        switch (newPacket.get_Command())
        {
            case commands::UPLOAD:
                recvClientUpload(newPacket);
                snprintf(response, sizeof(response), "File '%s' successfully uploaded on the ftp-server", newPacket.get_FileName());
                ::send(client, response, 2048, 0);
                break;
            case commands::DOWNLOAD:
                char filePath[256];
                snprintf(filePath, sizeof(filePath), "data/%s/%s", newPacket.get_Username(), newPacket.get_FileName());
                std::cout << "filepath: " << filePath << std::endl;
                sendFile(client, filePath, newPacket.get_Username());
                break;
            case commands::DELETE:
                deleteFile(newPacket.get_FileName(), newPacket.get_Username());
                snprintf(response, sizeof(response), "File '%s' successfully deleted on the ftp-server", newPacket.get_FileName());
                ::send(client, response, 2048, 0);
                break;
            default:
                std::cerr << "Error: Invalid command" << std::endl;
                break;
        }
    };

    std::thread clientThread(packetParsing, clientFD);
    std::cout << "ID " << clientFD << ": Done" << std::endl;

    clientThread.join();
    return 0;
}

// void Server::parsePacket(int clientFD) 
// {
//     char buffer[MAX_SIZE_PACKET];
//     ::recv(clientFD, buffer, MAX_SIZE_PACKET, 0);

//     FTP_Packet newPacket = *(FTP_Packet*) buffer;

//     if (strcmp(newPacket.get_Command(), "-upload") == 0)
//     {
//         recvClientUpload(newPacket);
//     }
//     else if (strcmp(newPacket.get_Command(), "-download") == 0)
//     {
        
//     }
//     else if (strcmp(newPacket.get_Command(), "-delete") == 0)
//     {
        
//     }
//     else
//     {
//         std::cerr << "Error: Invalid command" << std::endl;
//     }
// }

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