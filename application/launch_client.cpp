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

    char username[64];
    int port = 0;
    char serverIP[INET_ADDRSTRLEN];
    
    ftpServer = std::strtok(ftpServer, "@");
    strcpy(username, ftpServer);

    ftpServer = std::strtok(NULL, ":");
    strcpy(serverIP, ftpServer);

    ftpServer = std::strtok(NULL, " ");
    port = std::stoi(ftpServer);
    
    Client client = Client(serverIP, port);
    int clientSocket = client.get_socketFD();

    if (strcmp(command, "-upload") == 0)
    {
        std::cout << "Sending: " << fileName << std::endl;

        client.sendFile(clientSocket, fileName, username);
        
        char response[2048];
        client.recv(clientSocket, response, 0);
    }
    else if (strcmp(command, "-download") == 0)
    {
        std::cout << "Download: " << fileName << std::endl;

    }
    else if (strcmp(command, "-delete") == 0)
    {
        std::cout << "Delete: " << fileName << std::endl;

        if (client.deleteFile(fileName, username) == 0)
        {
            std::cout << "File deleted" << std::endl;
        }
        else
        {
            std::cerr << "Error: Could not delete file" << std::endl;
        }
    }
    else
    {
        std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-download|-upload|-delete> <filepath>" << std::endl;
        return -1;
    }
    
    // char message[1024] = "ping";
    // client.send(clientSocket, message, 0);
    // std::cout << "Message sent: " << message << std::endl;

    // client.recv(clientSocket, response, 0);
    // std::cout << "Server response: " << response << std::endl;

    return 0;
}