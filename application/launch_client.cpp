#include "../classes/include/client.hpp"

int main(int argc, char *argv[])
{
    if (argc == 0 && argc > 4)
    {
        std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-download|-upload|-delete> <filepath>" << std::endl;
        return -1;
    }
    char* ftpServer = argv[1];
    char* command = argv[2];
    char* fileName = argv[3];

    std::cout << "Command: " << command << std::endl;

    char username[64];
    int port = 0;
    char serverIP[INET_ADDRSTRLEN];
    char response[2048];

    
    ftpServer = std::strtok(ftpServer, "@");
    strcpy(username, ftpServer);

    ftpServer = std::strtok(NULL, ":");
    strcpy(serverIP, ftpServer);

    ftpServer = std::strtok(NULL, " ");
    port = std::stoi(ftpServer);
    
    Client client = Client(serverIP, port);
    int clientSocket = client.get_socketFD();

    FTP_Packet packet = FTP_Packet();

    if (strcmp(command, "-upload") == 0) 
    {
        packet.set_Command(commands::UPLOAD);
        std::cout << "Sending: " << fileName << std::endl;

        client.sendFile(clientSocket, fileName, username);
        ::recv(clientSocket, response, 2048, 0);
    } 
    else if (strcmp(command, "-download") == 0) 
    {
        packet.set_Command(commands::DOWNLOAD);
        packet.set_FileName(fileName);
        packet.set_Username(username);

        client.send(clientSocket, &packet, 0);
        client.recvServerDownload();
        std::cout << "Downloaded in '" << DESTINATION_PATH << "': " << fileName << std::endl;
    } 
    else if (strcmp(command, "-delete") == 0) 
    {
        packet.set_Command(commands::DELETE);
        packet.set_FileName(fileName);
        packet.set_Username(username);
        client.deleteFileOnServer(packet);
        ::recv(clientSocket, response, 2048, 0);
    }
    else
    {
        std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-download|-upload|-delete> <filepath>" << std::endl;
        return -1;
    }

    return 0;
}