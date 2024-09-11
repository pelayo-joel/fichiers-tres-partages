#include "../include/server.hpp"

Server::Server(int maxQueue) : FTP_Socket(PORT)
{
    // int serverSocket = this->get_socketFD();

    sinAddress_.sin_addr.s_addr = INADDR_ANY;
    char serverIP[INET_ADDRSTRLEN] = IP_ADDRESS;
    inet_ntop(AF_INET, &sinAddress_.sin_addr, serverIP, INET_ADDRSTRLEN);

    bind();
    listen(maxQueue);
    std::cout << "Servers listens on port " << PORT << std::endl;
    logger.EventLog(9, "Servers listens on port " + std::to_string(PORT));

}

Server::~Server() {}

Server::Server(const Server &base) : FTP_Socket(base) {}

Server &Server::operator=(const Server &base)
{
    if (this != &base)
    {
        FTP_Socket::operator=(base);
        // addrlen_ = base.addrlen_;
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

int Server::accept()
{
    int serverSocket = this->get_socketFD();
    int addrlen = sizeof(sinAddress_);
    return ::accept(serverSocket, (struct sockaddr *)&sinAddress_, (socklen_t *)&addrlen);
}


char* Server::createUserFolder(char* username) 
{
    std::string mainFolder = DESTINATION_PATH;
    std::string userFolder = mainFolder + username;

    if (!std::filesystem::exists(mainFolder))
    {
        std::filesystem::create_directory(mainFolder);
    }

    if (!std::filesystem::exists(userFolder))
    {
        std::filesystem::create_directory(userFolder);
    }

    auto strLen = userFolder.length();
    char* userFolderPath = new char[strLen + 2];
    std::memcpy(userFolderPath, userFolder.c_str(), strLen);

    userFolderPath[strLen] = '/';
    userFolderPath[strLen+1] = '\0';
    logger.EventLog(9, "User folder created: " + std::string(userFolderPath));
    return userFolderPath;
}

int Server::recvClientUpload(int socket, FTP_Packet packet)
{
    char filePath[MAX_SIZE_MESSAGE];

    char* userPath = createUserFolder(packet.get_Username());
    strcpy(filePath, pathToReceivedFile(userPath, packet.get_FileName()));

    RecvFile(socket, filePath, packet.get_FileSize());
    logger.EventLog(9, "File uploaded to server: " + std::string(filePath));
    return 0;
}

int Server::deleteFile(char* fileName, char* username) 
{
    char completePath[MAX_SIZE_MESSAGE];
    snprintf(completePath, sizeof(completePath), "%s%s/%s", DESTINATION_PATH, username, fileName);

    std::cout << "Deleting file complete path: " << completePath << std::endl;
    logger.EventLog(9, "File deleted: " + std::string(completePath));
    int status = remove(completePath);
    return status;
}

int Server::createClientThread(int clientFD)
{
    auto packetParsing = [this](int client) {
        char buffer[PACKET_SIZE];
        char response[MAX_SIZE_MESSAGE];
        char username[MAX_SIZE_USER];
        char password[MAX_SIZE_USER];
        int authenticationStatus = -1;
        int attempts = 0;
        char usernameCheck[MAX_SIZE_USER];

        ::recv(client, usernameCheck, MAX_SIZE_USER, 0);

        if (checkUserExists(usernameCheck) != 0)
        {
            char newPassword[MAX_SIZE_MESSAGE];

            ::send(client, "New user detected ! Please create a password : ", MAX_SIZE_MESSAGE, 0);
            ::recv(client, newPassword, MAX_SIZE_MESSAGE, 0);
            
            if (createNewUser(usernameCheck, newPassword) == 0) 
            {
                ::send(client, "User created successfully", MAX_SIZE_MESSAGE, 0);
            }
            else
            {
                ::send(client, "Error: User not created", MAX_SIZE_MESSAGE, 0);
                return -1;
            }
        }
        else
        {
            ::send(client, "OK", MAX_SIZE_MESSAGE, 0);
        }
        
        while (attempts < 3)
        {
            char bufferAuthentication[MAX_SIZE_BUFFER];
            
            std::cout << "Waiting for client " << client << " authentication" << std::endl;
            ::recv(client, bufferAuthentication, MAX_SIZE_BUFFER, 0);

            char* credentials = strdup(bufferAuthentication);

            credentials = std::strtok(credentials, ":");
            strcpy(username, credentials);
            credentials = std::strtok(NULL, "\0");
            strcpy(password, credentials);

            authenticationStatus = checkClientAuthentication(client, username, password);
            if (authenticationStatus == 0)
            {
                break;
            }
            
            attempts++;
        }
        
        if (attempts >= 3) 
        {
            std::cerr << "Error: Too many attempts" << std::endl;
            close(client);
            return -1;
        }

        ::recv(client, buffer, sizeof(FTP_Packet), 0);
        FTP_Packet newPacket = *(FTP_Packet*) buffer;

        FTP_Packet responsePacket = FTP_Packet();
        switch (newPacket.get_Command())
        {
            case command::UPLOAD:
                {
                    recvClientUpload(client, newPacket);
                    snprintf(response, sizeof(response), "File '%s' successfully uploaded on the ftp-server", newPacket.get_FileName());
                    ::send(client, response, MAX_SIZE_MESSAGE, 0);
                    logger.EventLog(9, "File uploaded to server: " + std::string(response));
                    break;
                }
            case command::DOWNLOAD:
                {
                    char filePath[MAX_SIZE_MESSAGE];
                    snprintf(filePath, sizeof(filePath), "data/%s/%s", newPacket.get_Username(), newPacket.get_FileName());
                    const int64_t fileSize = getFileSize(filePath);
                    responsePacket.set_FileName(filePath);
                    responsePacket.set_FileSize(fileSize);
                    ::send(client, &responsePacket, sizeof(FTP_Packet), 0);
                    sendFile(client, filePath, fileSize);
                    logger.EventLog(9, "File send to the client: " + std::string(filePath));
                    break;
                }   
            case command::DELETE:
                deleteFile(newPacket.get_FileName(), newPacket.get_Username());
                snprintf(response, sizeof(response), "File '%s' successfully deleted on the ftp-server", newPacket.get_FileName());
                ::send(client, response, MAX_SIZE_MESSAGE, 0);
                logger.EventLog(9, "File deleted: " + std::string(response));

                break;
            default:
                std::cerr << "Error: Invalid command" << std::endl;
                break;
        }
        std::cout << "ID " << client << ": Done" << std::endl;
        return 0;
    };

    std::thread clientThread(packetParsing, clientFD);
    std::move(clientThread).detach();
    logger.EventLog(9, "Client thread started: " + std::to_string(clientFD));

    return 0;
}

int Server::checkUserExists(char* username) {
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
            int lastChar = strlen(passwordLine) - 1;
            passwordLine[lastChar] = '\0';

            if (strcmp(usernameLine, username) == 0)
            {
                file.close();
                return 0;
            }
        }

        file.close();
        return -1;
    }
    else
    {
        std::cerr << "Error: File not found" << std::endl;
        return -1;
    }

    return 0;
}

void Server::handleLeave(int signal) {
    std::cout << "\nSignal (" << signal << ") reçu. Exécution du code avant l'arrêt du programme..." << std::endl;

    AppLogs& logger = AppLogs::Instance();
    logger.EventLog(9, "Leaving...");
    logger.GenerateFile();
    exit(signal);  // Quitter le programme
}

int Server::createNewUser(char* username, char* password) {
    std::ofstream file;
    char filePath[MAX_SIZE_MESSAGE];

    snprintf(filePath, sizeof(filePath), "%s/very_safe_trust_me_bro.txt", DESTINATION_PATH);
    file.open(filePath, std::ios::app);

    std::cout << "Creating new user: " << username << ", password: " << password << std::endl;

    if (file.is_open()) 
    {
        file << username << ":" << password << std::endl;
        file.close();
        createUserFolder(username);
    }
    else
    {
        std::cerr << "Error: File not found" << std::endl;
        return -1;
    }

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

        file.close();
    }
    else
    {
        std::cerr << "Error: File not found" << std::endl;
        return -1;
    }

    return 0;
}