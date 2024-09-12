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
        char response[MAX_SIZE_BUFFER];
        char username[MAX_SIZE_USER];
        char password[MAX_SIZE_USER];
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
                break;
            }
            case command::DOWNLOAD:
            {
                if (sendClientDownload(client, newPacket) != 0)
                {
                    return -1;
                }
                snprintf(response, sizeof(response), "File '%s' successfully downloaded from server", newPacket.get_FileName());
                break;
            }
            case command::DELETE:
            {
                if (deleteFile(client, newPacket.get_Path(), newPacket.get_Username()) != 0)
                {
                    return -1;
                }
                snprintf(response, sizeof(response), "File '%s' successfully deleted on the ftp-server", newPacket.get_FileName());
                break;
            }
            case command::LIST:
            {
                displayList(client, newPacket.get_Username(), newPacket.get_Path());
                break;
            }
            case command::CREATE: 
            {
                char* folderStatus = createFolder(newPacket.get_Username(), newPacket.get_FolderName(), newPacket.get_Path());
                if (folderStatus == nullptr)
                {
                    snprintf(response, sizeof(response), "Error: Folder '%s' can't be created on the ftp-server", newPacket.get_FolderName());
                    send(client, response);
                    return -1;
                }
                snprintf(response, sizeof(response), "Folder '%s' successfully created on the ftp-server", newPacket.get_FolderName());
                break;
            }
            case command::RM:
            {
                deleteFolder(client, newPacket.get_Username(), newPacket.get_Path());
                snprintf(response, sizeof(response), "Folder '%s' successfully deleted on the ftp-server", newPacket.get_Path());
                break; 
            }
            case command::RENAME:
            {
                renameFolder(client, newPacket.get_Username(), newPacket.get_Path(), newPacket.get_FolderName());
                snprintf(response, sizeof(response), "Folder '%s' successfully renamed to '%s' on the ftp-server", newPacket.get_Path(), newPacket.get_FolderName());
                break;
            }
            default:
                std::cerr << "Error: Invalid command" << std::endl;
                logger.EventLog(WARNING, "{" + std::string(username) + "} " + "Error: Invalid command");
                break;
            }
        
        responsePacket.set_Message(response);
        send(client, &responsePacket);
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
    char newPassword[MAX_SIZE_BUFFER];
    char response[MAX_SIZE_BUFFER];

    if (checkUserExists(username) != 0)
    {
        snprintf(response, sizeof(response), "New user detected ! Please create a password : ");
        send(clientFD, response);
        logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "[SEND] : New user detected ! Creating password...");
        recv(clientFD, newPassword);
        
        if (createNewUser(username, newPassword) == 0) 
        {
            snprintf(response, sizeof(response), "User created successfully");
            send(clientFD, response);
            logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "[SEND] : User created successfully");
        }
        else
        {
            snprintf(response, sizeof(response), "Error: User not created");
            send(clientFD, response);
            logger.EventLog(WARNING, "{" + std::string(username) + "} " + "[SEND] : Error: User not created");
        }
    }
    else
    {
        snprintf(response, sizeof(response), "OK");
        send(clientFD, response);
    }
}

int Server::checkUserExists(char* username) 
{
    std::ifstream file;
    std::string line;
    char filePath[MAX_SIZE_BUFFER];

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
    char filePath[MAX_SIZE_BUFFER];

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
    int attempts = 0;
    while (attempts < 3)
    {
        char bufferAuthentication[PACKET_SIZE];
        
        std::cout << "Waiting for client (" << clientFD << ") " << username << " authentication" << std::endl;
        recv(clientFD, bufferAuthentication);

        char* credentials = strdup(bufferAuthentication);

        credentials = std::strtok(credentials, ":");
        strcpy(username, credentials);
        credentials = std::strtok(NULL, "\0");
        strcpy(password, credentials);

        logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Waiting for authentication...");

        int authenticationStatus = checkClientAuthentication(clientFD, username, password);
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

        return -1;
    }

    return 0;
}

int Server::checkClientAuthentication(int client, char* username, char* password) {
    std::ifstream file;
    std::string line;
    char filePath[MAX_SIZE_BUFFER];

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
                ::send(client, "OK", MAX_SIZE_BUFFER, 0);
                file.close();
                return 0;
            }
            else if (strcmp(usernameLine, username) == 0 && strcmp(passwordLine, password) != 0) 
            {
                ::send(client, "Invalid password", MAX_SIZE_BUFFER, 0);
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
    char response[MAX_SIZE_BUFFER];

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
            snprintf(response, sizeof(response), "Error: Could not create or open file on server");
            send(socket, response);
            return statusRecvFile;
        case -2:
            std::cerr << "Error: Could not write to file " << filePath << std::endl;
            logger.EventLog(ERROR, "{" + std::string(packet.get_Username()) + "} " + "Error: Could not write to file " + std::string(filePath));
            snprintf(response, sizeof(response), "Error: During writing/receiving to file on server");
            send(socket, response);
            return statusRecvFile;
        default:
            break;
    }
    return 0;
}

int Server::sendClientDownload(int clientFD, FTP_Packet packet)
{
    char filePath[MAX_SIZE_PATH];
    char response[MAX_SIZE_BUFFER];
    FTP_Packet fileHeader = FTP_Packet();

    snprintf(filePath, sizeof(filePath), "data/%s/%s", packet.get_Username(), packet.get_FileName());
    const int64_t fileSize = getFileSize(filePath);
    fileHeader.set_FileName(filePath);
    fileHeader.set_FileSize(fileSize);
    send(clientFD, &fileHeader);

    int statusSendFile = sendFile(clientFD, filePath, fileSize);
    switch (statusSendFile)
    {
    case 0:
        logger.EventLog(DEBUG, "{" + std::string(packet.get_Username()) + "} " + "File send to the client: " + std::string(filePath));
        break;
    case -1:
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        logger.EventLog(ERROR, "{" + std::string(packet.get_Username()) + "} " + "Error: Could not create or open file " + std::string(filePath));
        snprintf(response, sizeof(response), "Error: Could not open file on server");
        send(clientFD, response);
        return statusSendFile;
    case -2:
        std::cerr << "Error: Could not read into file " << filePath << std::endl;
        logger.EventLog(ERROR, "{" + std::string(packet.get_Username()) + "} " + "Error: Could not read into file " + std::string(filePath));
        snprintf(response, sizeof(response), "Error: During file reading/sending on server");
        send(clientFD, response);
        return statusSendFile;
    default:
        break;
    }
    return 0;
}

int Server::deleteFile(int clientFD, char* fileName, char* username) 
{
    char completePath[MAX_SIZE_PATH];
    char response[MAX_SIZE_BUFFER];
    snprintf(completePath, sizeof(completePath), "%s%s/%s", DESTINATION_PATH, username, fileName);

    int status = remove(completePath);
    switch (status)
    {
    case 0:
        logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "File deleted: " + std::string(completePath));    
        break;
    default:
        std::cerr << "Error: Could not delete file" << completePath << std::endl;
        logger.EventLog(ERROR, "{" + std::string(username) + "} " + "Error: Could not delete file " + std::string(completePath));
        snprintf(response, sizeof(response), "Error: Could not delete file on server");
        send(clientFD, response);
        return status;
    }
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
    logger.EventLog(DEBUG, "{" + std::string(username) + "} " + " Deleting folder: " + std::string(completePath));
}

void Server::renameFolder(int client, char* username, const char* oldPath, const char* newFolderName) 
{
    std::string completePath = std::string(DESTINATION_PATH) + username + "/" + oldPath;
    std::size_t found = completePath.find_last_of("/\\");
    std::string newPath = completePath.substr(0, found) + "/" + std::string(newFolderName);
    std::cout << "New path: " << newPath << std::endl;

    std::filesystem::rename(completePath, newPath);
    logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Rename folder " + std::string(oldPath) + " to " + std::string(newFolderName));
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
            logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Folder " + std::string(checkPath) + " can't be created !");
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

    logger.EventLog(DEBUG, "{" + std::string(username) + "} " + "Folder " + std::string(completePathFolder) + " successfully created on the ftp-server");

    return completePathFolder;

}

