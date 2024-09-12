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
    logger.EventLog(DEBUG, "Servers listens on port " + std::to_string(PORT));

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


int Server::createClientThread(int clientFD)
{
    auto packetParsing = [this](int client) {
        char buffer[PACKET_SIZE];
        char response[MAX_SIZE_MESSAGE];
        char username[MAX_SIZE_USER];
        char password[MAX_SIZE_USER];
        int authenticationStatus = -1;
        int attempts = 0;
        // char usernameCheck[MAX_SIZE_USER];

        recv(client, username);
        userRecognition(client, username);
        logger.EventLog(DEBUG, "Client thread for user : " + std::string(username) + " created.");
        
        if (waitingUserAuthentication(client, username, password) != 0) 
        {
            return -1;
        }
        
        recv(client, buffer);
        FTP_Packet newPacket = *(FTP_Packet*) buffer;
        FTP_Packet responsePacket = FTP_Packet();

        std::cout << username << " - Command: " << newPacket.get_Command() << std::endl;

        switch (newPacket.get_Command())
        {
            case command::UPLOAD:
                {
                    if (recvClientUpload(client, newPacket) != 0) 
                    {
                        return -1;
                    }
                    snprintf(response, sizeof(response), "File '%s' successfully uploaded on the ftp-server", newPacket.get_FileName());
                    responsePacket.set_Message(response);
                    break;
                }
            case command::DOWNLOAD:
                {
                    char filePath[MAX_SIZE_MESSAGE];
                    snprintf(filePath, sizeof(filePath), "data/%s/%s", newPacket.get_Username(), newPacket.get_FileName());
                    const int64_t fileSize = getFileSize(filePath);
                    responsePacket.set_FileName(filePath);
                    responsePacket.set_FileSize(fileSize);
                    send(client, &responsePacket);
                    sendFile(client, filePath, fileSize);
                    logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "File send to the client: " + std::string(filePath));
                    break;
                }   
            case command::DELETE:
                deleteFile(newPacket.get_Path(), newPacket.get_Username());
                snprintf(response, sizeof(response), "File '%s' successfully deleted on the ftp-server", newPacket.get_FileName());
                ::send(client, response, MAX_SIZE_MESSAGE, 0);
                logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "File deleted: " + std::string(response));

                break;
            case command::LIST:
                displayList(client, newPacket.get_Username(), newPacket.get_Path());
                break;
            case command::CREATE: 
            {
                char* folderStatus = createFolder(newPacket.get_Username(), newPacket.get_FolderName(), newPacket.get_Path());
                if (folderStatus == nullptr)
                {
                    snprintf(response, sizeof(response), "Folder '%s' can't be created !", newPacket.get_FolderName());
                    logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Folder " + std::string(newPacket.get_FolderName()) + " can't be created !");

                }
                else 
                {
                    snprintf(response, sizeof(response), "Folder '%s' successfully created on the ftp-server", newPacket.get_FolderName());
                    logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Folder " + std::string(newPacket.get_FolderName()) + " successfully created on the ftp-server");

                }
                ::send(client, response, MAX_SIZE_MESSAGE, 0);
                break;
            }
            case command::RM:
                deleteFolder(client, newPacket.get_Username(), newPacket.get_Path());
                logger.EventLog(DEBUG, "{" + std::string(username) + "} " + " Deleting folder: " + std::string(newPacket.get_Path()));

                break; 
            case command::RENAME:
            {
                renameFolder(client, newPacket.get_Username(), newPacket.get_Path(), newPacket.get_FolderName());
                ::send(client, response, MAX_SIZE_MESSAGE, 0);
                logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Rename folder " + std::string(newPacket.get_Path()) + " to " + std::string(newPacket.get_FolderName()));

                break;
            }
                break;
            default:
                std::cerr << "Error: Invalid command" << std::endl;
                logger.EventLog(WARNING, "{" + std::string(username) + "} " + "Error: Invalid command");
                break;


            }
        
         send(client, responsePacket);

        std::cout << "ID " << client << ": Done" << std::endl;
        logger.EventLog(DEBUG, "Client thread for user : " + std::string(username) + " ended.");

        return 0;
    };

    std::thread clientThread(packetParsing, clientFD);
    std::move(clientThread).detach();


    return 0;
}

void Server::userRecognition(int clientFD, char* username) 
{
    if (checkUserExists(username) != 0)
    {
        char newPassword[MAX_SIZE_MESSAGE];

        send(clientFD, "New user detected ! Please create a password : ");
        logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "[SEND] : New user detected ! Creating password...");
        recv(clientFD, newPassword);
        
        if (createNewUser(username, newPassword) == 0) 
        {
            send(clientFD, "User created successfully");
            logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "[SEND] : User created successfully");
        }
        else
        {
            send(clientFD, "Error: User not created");
            logger.EventLog(WARNING, "{" + std::string(username) + "} " + "[SEND] : Error: User not created");
            return -1;
        }
    }
    else
    {
        send(clientFD, "OK");
    }
}

int Server::checkUserExists(char* username) 
{
    std::ifstream file;
    std::string line;
    char filePath[MAX_SIZE_MESSAGE];

    snprintf(filePath, sizeof(filePath), "%s/%s", DESTINATION_PATH, AUTHENTICATION_FILE);
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
        logger.EventLog(WARNING, "{" + std::string(username) + "} " + "Error: File not found");
        return -1;
    }

    return 0;
}

int Server::createNewUser(char* username, char* password) 
{
    std::ofstream file;
    char filePath[MAX_SIZE_MESSAGE];

    snprintf(filePath, sizeof(filePath), "%s/very_safe_trust_me_bro.txt", DESTINATION_PATH);
    file.open(filePath, std::ios::app);

    std::cout << "Creating new user: " << username << ", password: " << password << std::endl;
	logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Creating new user...");
    if (file.is_open()) 
    {
        file << username << ":" << password << std::endl;
        file.close();
        createFolder(username, "", "");
    }
    else
    {
        std::cerr << "Error: File not found" << std::endl;
        logger.EventLog(WARNING, "Error: File "+ std::string(filePath)  +" not found");
        return -1;
    }

    return 0;
}

int Server::waitingUserAuthentication(int clientFD, char* username, char* password) 
{
    while (attempts < 3)
    {
        char bufferAuthentication[MAX_SIZE_BUFFER];
        
        std::cout << "Waiting for client (" << clientFD << ") " << username << " authentication" << std::endl;
        recv(clientFD, bufferAuthentication);

        char* credentials = strdup(bufferAuthentication);

        credentials = std::strtok(credentials, ":");
        strcpy(username, credentials);
        credentials = std::strtok(NULL, "\0");
        strcpy(password, credentials);

        logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Waiting for authentication...");

        authenticationStatus = checkClientAuthentication(clientFD, username, password);
        if (authenticationStatus == 0)
        {
            break;
        }
        
        attempts++;
    }
    
    if (attempts >= 3) 
    {
        std::cerr << "Error: Too many attempts" << std::endl;
        logger.EventLog(WARNING, "{" + std::string(username) + "} " + "Error: Too many attempts");

        close(client);
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
                logger.EventLog(WARNING, "{" + std::string(username) + "} " + "[SEND] : Invalid password");

                file.close();
                return -1;
            }
        }

        file.close();
    }
    else
    {
        std::cerr << "Error: File not found" << std::endl;
        logger.EventLog(WARNING, "Error: File "+ std::string(filePath)  +" not found");
        return -1;
    }

    return 0;
}


int Server::recvClientUpload(int socket, FTP_Packet packet)
{
    char filePath[MAX_SIZE_PATH];

    char* userPath = createFolder(packet.get_Username(), "", "");
    strcpy(filePath, pathToReceivedFile(userPath, packet.get_FileName()));

    int statusRecvFile = recvFile(socket, filePath, packet.get_FileSize());

    switch (statusRecvFile)
    {
        case 0:
            logger.EventLog(DEBUG, "{" + std::string(packet.get_Username()) + "} " + "File uploaded to server: " + std::string(filePath));
            break;
        case -1:
            std::cerr << "Error: Could not create or open file " << filePath << std::endl;
            logger.EventLog(ERROR, "{" + std::string(packet.get_Username()) + "} " + "Error: Could not create or open file " + std::string(filePath));
            send(socket, "Error: Could not create or open file");
            return statusRecvFile;
        case -2:
            std::cerr << "Error: Could not write to file " << filePath << std::endl;
            logger.EventLog(ERROR, "{" + std::string(packet.get_Username()) + "} " + "Error: Could not write to file " + std::string(filePath));
            send(socket, "Error: Could not write to file");
            return statusRecvFile;
        default:
            break;
    }
    return 0;
}

int Server::sendClientDownload(int clientFD, FTP_Packet packetReceived, FTP_Packet responsePacket)
{
    char filePath[MAX_SIZE_PATH];
    char response[MAX_SIZE_MESSAGE];

    snprintf(filePath, sizeof(filePath), "data/%s/%s", packetReceived.get_Username(), packetReceived.get_FileName());
    const int64_t fileSize = getFileSize(filePath);
    responsePacket.set_FileName(filePath);
    responsePacket.set_FileSize(fileSize);
    send(clientFD, &responsePacket);
    sendFile(clientFD, filePath, fileSize);
    logger.EventLog(DEBUG, "{" + std::string(packetReceived.get_Username()) + "} " + "File send to the client: " + std::string(filePath));

    return 0;
}

int Server::deleteFile(char* fileName, char* username) 
{
    char completePath[MAX_SIZE_MESSAGE];
    snprintf(completePath, sizeof(completePath), "%s%s/%s", DESTINATION_PATH, username, fileName);

    std::cout << "Deleting file complete path: " << completePath << std::endl;
    logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "File deleted: " + std::string(completePath));
    int status = remove(completePath);
    return status;
}

void Server::displayList(int client, char* username, const char* path) 
{
    std::string rootList = std::string(DESTINATION_PATH) + username;
    std::cout << "path: " << path << std::endl;
    if (strcmp(path, "") != 0)
    {
        rootList += std::string("/") + std::string(path);
    }

    char list[MAX_SIZE_BUFFER] = "";

    for (const auto & entry : std::filesystem::directory_iterator(rootList))
    {
        std::string entryPath = entry.path().string();
        entryPath += "\n";
        entryPath = entryPath.substr(rootList.length() + 1, entryPath.length());
        strcat(list, entryPath.c_str()); 
    }
    ::send(client, list, MAX_SIZE_BUFFER, 0);

}

void Server::deleteFolder(int client, char* username, const char* path) 
{
    std::string completePath = std::string(DESTINATION_PATH) + username + "/" + path;
    std::filesystem::remove_all(completePath);

    ::send(client, "Folder deleted", MAX_SIZE_MESSAGE, 0);
}

int Server::renameFolder(int client, char* username, const char* oldPath, const char* newFolderName) 
{
    std::string completePath = std::string(DESTINATION_PATH) + username + "/" + oldPath;
    std::size_t found = completePath.find_last_of("/\\");
    std::string newPath = completePath.substr(0, found) + "/" + std::string(newFolderName);
    std::cout << "New path: " << newPath << std::endl;

    std::filesystem::rename(completePath, newPath);
    ::send(client, "Folder renamed", MAX_SIZE_MESSAGE, 0);
    return 0;
}


void Server::handleLeave(int signal) {
    std::cout << "\nCréation du fichier de log..." << std::endl;

    AppLogs& logger = AppLogs::Instance();
    logger.EventLog(9, "Leaving...");
    logger.GenerateFile();
    exit(signal);  // Quitter le programme
}

char* Server::createFolder(char* username, const char* foldername, const char* path) 
{
    std::string mainFolder = DESTINATION_PATH;
    std::string newFolder;
    std::string userFolder = std::string(DESTINATION_PATH) + username + "/";

    if (strcmp(foldername, "") != 0 && strcmp(path, "") == 0) 
    {
        newFolder = userFolder + std::string(foldername) + std::string("/");
    }
    else if (strcmp(foldername, "") != 0 && strcmp(path, "") != 0) 
    {
        std::string checkPath = userFolder + std::string(path);
        if (std::filesystem::exists(checkPath)) 
        {
            newFolder = userFolder + std::string(path) + std::string("/") + std::string(foldername) + std::string("/");
            std::cout << "New folder: " << newFolder << std::endl;
        }
        else 
        {
            // PATH DOES NOT EXIST
            return nullptr;
        }
    }

    if (!std::filesystem::exists(mainFolder))
    {
        std::filesystem::create_directory(mainFolder);
    }


    if (!std::filesystem::exists(userFolder))
    {
        std::filesystem::create_directory(userFolder);

        auto strLen = userFolder.length();
        char* userFolderPath = new char[strLen + 1];
        std::memcpy(userFolderPath, userFolder.c_str(), strLen);

        // userFolderPath[strLen] = '/';
        userFolderPath[strLen] = '\0';
        logger.EventLog(DEBUG, "User folder created: " + std::string(userFolderPath));
    }

    std::cout << "User folder: " << userFolder << std::endl;
    if (strcmp(foldername, "") != 0) 
    {
        if (!std::filesystem::exists(newFolder)) 
        {
            std::filesystem::create_directory(newFolder);
        }
    }
    
    char* completePathFolder = new char[userFolder.size() + 1];
    std::strcpy(completePathFolder, userFolder.c_str());

    logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "User folder created: " + std::string(completePathFolder));

    return completePathFolder;

}

