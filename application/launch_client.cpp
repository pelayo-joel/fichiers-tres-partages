#include "../classes/include/client.hpp"

int clientAuthentication(int clientSocket, char* username)
{
    int attempts = 0;
    char response[MAX_SIZE_MESSAGE];
    char userAuthentication[MAX_SIZE_USER * 2 + 1];

    ::send(clientSocket, username, MAX_SIZE_USER, 0);
    ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);

    if (strcmp(response, "OK") != 0)
    {
        char password[MAX_SIZE_MESSAGE];
        std::cout << response;
        std::cin >> password;

        std::cout << password << std::endl;
        ::send(clientSocket, password, MAX_SIZE_MESSAGE, 0);
        ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);
    }
    else
    {
        std::cout << "Welcome back " << username << std::endl;
    }


    while (attempts < 3)
    {
        char password[MAX_SIZE_USER];
        strcpy(response, "");
        std::cout << "Please enter your password: ";
        std::cin >> password;

        snprintf(userAuthentication, sizeof(userAuthentication),"%s:%s", username, password);
        ::send(clientSocket, userAuthentication, MAX_SIZE_MESSAGE, 0);
        ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);
        std::cout << response << std::endl;
        if (strcmp(response, "OK") == 0)
        {
            break;
        }
        
        attempts++;
    }

    if (attempts >= 3)
    {
        std::cerr << "Error: Authentication failed, bye, remember your password next time !" << std::endl;
        return -1;
    }

    return 0;
}
    
int main(int argc, char *argv[])
{
    if (argc == 0 && argc > 4)
    {
        std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-download|-upload|-delete> <filepath>" << std::endl;
        return -1;
    }
    char* ftpServer = argv[1];
    char* command = argv[2];
    char* fileName;
    char* folderName;
    char* path;

    // if (argc == 5) 
    // {
    //     path = argv[4];
    // }
    
    char username[MAX_SIZE_USER];
    int port = 0;
    char serverIP[INET_ADDRSTRLEN];
    char response[MAX_SIZE_MESSAGE];

    ftpServer = std::strtok(ftpServer, "@");
    strcpy(username, ftpServer);

    ftpServer = std::strtok(NULL, ":");
    strcpy(serverIP, ftpServer);

    ftpServer = std::strtok(NULL, " ");
    port = std::stoi(ftpServer);
    
    Client client = Client(serverIP, port);
    int clientSocket = client.get_socketFD();

    FTP_Packet packet = FTP_Packet();

    if (clientAuthentication(clientSocket, username) != 0)
    {
        close(clientSocket);
        return -1;
    }


    if (strcmp(command, "-upload") == 0) 
    {
        fileName = argv[3];
        packet.set_Command(commands::UPLOAD);
        packet.set_FileName(fileName);

        client.sendFile(clientSocket, fileName, username);
        ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);
    } 
    else if (strcmp(command, "-download") == 0) 
    {
        fileName = argv[3];
        packet.set_Command(commands::DOWNLOAD);
        packet.set_FileName(fileName);
        packet.set_Username(username);

        client.send(clientSocket, &packet, 0);
        client.recvServerDownload();
        std::cout << "Downloaded in '" << DESTINATION_PATH << "': " << fileName << std::endl;
    } 
    else if (strcmp(command, "-delete") == 0) 
    {
        fileName = argv[3];
        packet.set_Command(commands::DELETE);
        packet.set_FileName(fileName);
        packet.set_Username(username);
        client.sendPacket(packet);

        ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);
        std::cout << response << std::endl;
    }
    else if (strcmp(command, "-list") == 0) 
    {
        char listMessage[MAX_SIZE_BUFFER];
        if (argc == 4)
        {
            path = argv[3];
        }
        packet.set_Command(commands::LIST);
        packet.set_Username(username);
        if (path != nullptr && strlen(path) > 0)
        {
            packet.set_Path(path);
        }

        client.sendPacket(packet);

        ::recv(clientSocket, listMessage, MAX_SIZE_BUFFER, 0);
        std::cout << listMessage << std::endl;
    }
    else if (strcmp(command, "-create") == 0) 
    {
        folderName = argv[3];
        if (argc == 5)
        {
            path = argv[4];
        }

        if (strcmp(folderName, "") != 0)
        {
            packet.set_Command(commands::CREATE);
            packet.set_Username(username);
            packet.set_FolderName(folderName);
            if (path != nullptr && strlen(path) > 0)
            {
                packet.set_Path(path);
            }

            client.sendPacket(packet);
            ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);
            std::cout << response << std::endl;
        }
        else 
        {
            std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-create> <foldername> <path>" << std::endl;
            return -1;
        }
    }
    else
    {
        std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-download|-upload|-delete> <filepath>" << std::endl;
        return -1;
    }

    return 0;
}