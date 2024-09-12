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
    char filePath[MAX_SIZE_PATH];
    char buffer[PACKET_SIZE];

    ::recv(this->get_socketFD(), buffer, PACKET_SIZE, 0); 
    FTP_Packet packetDownload = *(FTP_Packet*) buffer;

    strcpy(filePath, DESTINATION_PATH);
    strcat(filePath, packetDownload.get_FileName());

    RecvFile(this->get_socketFD(), filePath, packetDownload.get_FileSize());

    return 0;
}

int Client::userAuthentication(char* username)
{
    int attempts = 0;
    int clientSocket = this->get_socketFD();
    char response[MAX_SIZE_BUFFER];
    char userAuthentication[MAX_SIZE_USER * 2 + 1];

    userRecognition(username);

    while (attempts < 3)
    {
        char password[MAX_SIZE_USER];
        strcpy(response, "");
        std::cout << "Please enter your password: ";
        std::cin >> password;

        snprintf(userAuthentication, sizeof(userAuthentication),"%s:%s", username, password);
        send(clientSocket, userAuthentication);
        recv(clientSocket, response);
        if (strcmp(response, "OK") == 0)
        {
            std::cout << username << ", authentication succeeded"<< std::endl;
            break;
        }
        
        std::cout << response << std::endl;
        attempts++;
    }

    if (attempts >= 3)
    {
        std::cerr << "Error: Authentication failed, bye, remember your password next time !" << std::endl;
        return -1;
    }

    return 0;
}

void Client::userRecognition(char* username)
{
    int clientSocket = this->get_socketFD();
    char response[MAX_SIZE_BUFFER];

    send(clientSocket, username);
    recv(clientSocket, response);

    if (strcmp(response, "OK") != 0)
    {
        char password[MAX_SIZE_USER];
        std::cout << response;
        std::cin >> password;

        send(clientSocket, password);
        recv(clientSocket, response);
    }
    else
    {
        std::cout << "Welcome back " << username << std::endl;
    }
}