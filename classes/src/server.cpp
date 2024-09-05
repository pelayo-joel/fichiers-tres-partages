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
        addrlen_ = base.addrlen_;
        // threads_ = base.threads_;
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
        char response[MAX_SIZE_MESSAGE];
        char username[MAX_SIZE_USER];
        char password[MAX_SIZE_USER];
        int authenticationStatus = -1;


        while (authenticationStatus != 0)
        {
            char bufferAuthentication[MAX_SIZE_BUFFER];
            char incorrectPassword[MAX_SIZE_MESSAGE] = "Invalid password !";
            
            std::cout << "Waiting for client authentication" << std::endl;
            ::recv(client, bufferAuthentication, MAX_SIZE_BUFFER, 0);

            char* credentials = strdup(bufferAuthentication);

            credentials = std::strtok(credentials, ":");
            strcpy(username, credentials);
            credentials = std::strtok(NULL, "\0");
            strcpy(password, credentials);

            authenticationStatus = checkClientAuthentication(client, username, password);
            std::cout << "Authentication status: " << authenticationStatus << std::endl;
        }
        
        recv(client, buffer, 0);
        FTP_Packet newPacket = *(FTP_Packet*) buffer;

        switch (newPacket.get_Command())
        {
            case commands::UPLOAD:
                recvClientUpload(newPacket);
                snprintf(response, sizeof(response), "File '%s' successfully uploaded on the ftp-server", newPacket.get_FileName());
                ::send(client, response, MAX_SIZE_MESSAGE, 0);
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
                ::send(client, response, MAX_SIZE_MESSAGE, 0);
                break;
            default:
                std::cerr << "Error: Invalid command" << std::endl;
                break;
        }
        std::cout << "ID " << client << ": Done" << std::endl;
    };

    std::thread clientThread(packetParsing, clientFD);
    std::move(clientThread).detach();
    // clientThread.detach();
    // threads_.push(clientThread);

    return 0;
}

int Server::checkClientAuthentication(int client, char* username, char* password) {
    std::ifstream file;
    std::string line;
    char filePath[MAX_SIZE_MESSAGE];

    snprintf(filePath, sizeof(filePath), "%s/very_safe_trust_me_bro.txt", DESTINATION_PATH);
    file.open(filePath, std::ios::in);

    if (file.is_open()) 
    {
        while(std::getline(file, line)) 
        {
            char* buffer = (char*) line.c_str();
            char usernameLine[MAX_SIZE_USER];
            char passwordLine[MAX_SIZE_USER]; 

            buffer = std::strtok(buffer, ":");
            strcpy(usernameLine, buffer);
            buffer = std::strtok(NULL, "");
            strcpy(passwordLine, buffer);

            if (strcmp(usernameLine, username) == 0 && strcmp(passwordLine, password) == 0)
            {
                ::send(client, "OK", MAX_SIZE_MESSAGE, 0);
                file.close();
                return 0;
            }
            else if (strcmp(usernameLine, username) == 0 && strcmp(passwordLine, password) != 0) 
            {
                ::send(client, "Invalid password", MAX_SIZE_MESSAGE, 0);            
                file.close();
                return -1;
            }
        }
        std::cout << "User not found" << std::endl;
        file.close();
        if (createNewUser(username, password, filePath) == 0) {
            ::send(client, "User not found, new account created with the entered password", MAX_SIZE_MESSAGE, 0);
            std::cout << "User not found, new account created with the entered password" << std::endl;
        }
        else
        {
            return -1;
        }
        
    }
    else
    {
        std::cerr << "Error: File not found" << std::endl;
        return -1;
    }

    return 0;
}

int Server::createNewUser(char* username, char* password, char* authPath) {
    std::ofstream file;
    std::string line;

    file.open(authPath, std::ios::app);

    if (file.is_open()) 
    {
        file << username << ":" << password << std::endl;
        file.close();
    }
    else
    {
        std::cerr << "Error: File not found" << std::endl;
        return -1;
    }

    return 0;
}

int Server::accept()
{
    int serverSocket = this->get_socketFD();
    int addrlen = sizeof(sinAddress_);
    return ::accept(serverSocket, (struct sockaddr *)&sinAddress_, (socklen_t *)&addrlen);
}

int Server::getAddrlen() { return addrlen_; }
// std::queue<std::thread> Server::getClients() { return threads_; }

void Server::setAddrlen(int addrlen) { addrlen_ = addrlen; }
// void Server::setClientsThread(std::queue<std::thread> threadQueue) { threads_ = threadQueue; }
