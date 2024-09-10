#include "../include/client.hpp"

Client::Client(char* serverAddress, int port) : FTP_Socket(port)
{
    char serverIP[INET_ADDRSTRLEN];
    strcpy(serverIP, serverAddress);
    sinAddress_.sin_addr.s_addr = inet_addr(serverIP);

    ::inet_ntop(AF_INET, &sinAddress_.sin_addr, serverIP, INET_ADDRSTRLEN);
    connect();
}

Client::Client(const Client &base) : FTP_Socket(base) {}

Client::~Client() {}

Client &Client::operator=(const Client &base)
{
    if (this != &base)
    {
        FTP_Socket::operator=(base);
    }
    return *this;
}

int Client::connect()
{
    int clientSocket = this->get_socketFD();
    return ::connect(clientSocket, (struct sockaddr *)&sinAddress_, sizeof(sinAddress_));
}

int Client::recvServerDownload() 
{
    char filePath[MAX_SIZE_MESSAGE];
    char* buffer = recvFile(this->get_socketFD()); 
    std::ofstream file;

    FTP_Packet packetDownload = *(FTP_Packet*) buffer;

    strcpy(filePath, DESTINATION_PATH);
    strcat(filePath, packetDownload.get_FileName());

    file.open(filePath, std::ios::out);
    file.write(packetDownload.get_RawData(), packetDownload.get_FileSize());

    file.close();
    return 0;
}

int Client::deleteFileOnServer(FTP_Packet packet)
{
    send(this->get_socketFD(), &packet, 0);
    return 0;
}

