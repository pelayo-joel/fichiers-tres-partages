#include "../classes/include/client.hpp"

    
int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 5)
    {
        std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-download|-upload|-delete> <filepath>" << std::endl;
        return -1;
    }

    int port = 0;
    char* fileName;
    char* folderName;
    char* ftpServer = argv[1];
    char* command = argv[2];
    char path[MAX_SIZE_PATH] = "";
    char username[MAX_SIZE_USER];
    char serverIP[INET_ADDRSTRLEN];
    FTP_Packet responseBuffer = FTP_Packet();

    ftpServer = std::strtok(ftpServer, "@");
    strcpy(username, ftpServer);

    ftpServer = std::strtok(NULL, ":");
    strcpy(serverIP, ftpServer);

    ftpServer = std::strtok(NULL, " ");
    port = std::stoi(ftpServer);
    
    Client client = Client(serverIP, port);
    int clientSocket = client.get_socketFD();

    FTP_Packet packet = FTP_Packet();

    if (client.userAuthentication(username) != 0)
    {
        return -1;
    }

    packet.set_Username(username);

    if (strcmp(command, "-upload") == 0) 
    {
        fileName = argv[3];
        packet.set_Command(command::UPLOAD);
        packet.set_FileName(fileName);
        packet.set_FileSize(client.getFileSize(fileName));

        client.send(clientSocket, &packet);
        client.sendFile(clientSocket, fileName, packet.get_FileSize());
        client.recv(clientSocket, &responseBuffer);
    } 
    else if (strcmp(command, "-download") == 0) 
    {
        fileName = argv[3];
        packet.set_Command(command::DOWNLOAD);
        packet.set_FileName(fileName);

        client.send(clientSocket, &packet);
        client.recvServerDownload();
        client.recv(clientSocket, &responseBuffer);
        std::cout << "Downloaded in '" << DESTINATION_PATH << "': " << fileName << std::endl;
    } 
    else if (strcmp(command, "-delete") == 0) 
    {
        fileName = argv[3];
        packet.set_Path(fileName);
        packet.set_Command(command::DELETE);
        packet.set_FileName(fileName);

        client.send(clientSocket, &packet);
        client.recv(clientSocket, &responseBuffer);
    }
    else if (strcmp(command, "-list") == 0) 
    {
        if (argc == 4)
        {
            strcpy(path, argv[3]);
        }
        packet.set_Command(command::LIST);
        packet.set_Path(path);

        client.send(clientSocket, &packet);
        client.recv(clientSocket, &responseBuffer);
    }
    else if (strcmp(command, "-create") == 0) 
    {
        folderName = argv[3];
        if (argc == 5)
        {
            strcpy(path, argv[4]);
        }

        if (strcmp(folderName, "") != 0)
        {
            packet.set_Command(command::CREATE);
            packet.set_FolderName(folderName);
            packet.set_Path(path);

            client.send(clientSocket, &packet);
            client.recv(clientSocket, &responseBuffer);
        }
        else 
        {
            std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-create> <foldername> <path>" << std::endl;
            return -1;
        }
    }
    else if (strcmp(command, "-rm") == 0) 
    {
        if (argc == 4)
        {
            strcpy(path, argv[3]);
        }
        else 
        {
            std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-rm> <path>" << std::endl;
            return -1;
        }

        packet.set_Command(command::RM);
        packet.set_Path(path);
        
        client.send(clientSocket, &packet);
        client.recv(clientSocket, &responseBuffer);
    }
    else if (strcmp(command, "-rename") == 0)
    {
        folderName = argv[3];
        strcpy(path, argv[4]);
        
        packet.set_Command(command::RENAME);
        packet.set_FolderName(folderName);
        packet.set_Path(path);
        
        client.send(clientSocket, &packet);
        client.recv(clientSocket, &responseBuffer);
    }
    else
    {
        std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-download|-upload|-delete|-list|-create|-rm|-rename> <filename> <path>" << std::endl;
        return -1;
    }

    std::cout << responseBuffer.get_Message() << std::endl;

    return 0;
}