#include "../classes/include/client.hpp"

int clientAuthentication(int clientSocket, char* username)
{
    int attempts = 0;
    char password[MAX_SIZE_USER];
    char response[MAX_SIZE_MESSAGE];
    char userAuthentication[MAX_SIZE_USER * 2 + 1];
    
    do
    {
        std::cout << "Please enter your password: ";
        std::cin >> password;

        snprintf(userAuthentication, sizeof(userAuthentication),"%s:%s", username, password);
        ::send(clientSocket, userAuthentication, MAX_SIZE_MESSAGE, 0);
        ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);
        attempts++;
    } while (strcmp(response, "OK") != 0 && attempts < 3);


    if (attempts == 3)
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
    char* fileName = argv[3];

    std::cout << "Command: " << command << std::endl;

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
        return -1;
    }


    if (strcmp(command, "-upload") == 0) 
    {
        packet.set_Command(commands::UPLOAD);
        std::cout << "Sending: " << fileName << std::endl;

        client.sendFile(clientSocket, fileName, username);
        ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);
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
        ::recv(clientSocket, response, MAX_SIZE_MESSAGE, 0);
    }
    else
    {
        std::cerr << "Usage : " << argv[0] << " <username@ftp_serverIP:port> <-download|-upload|-delete> <filepath>" << std::endl;
        return -1;
    }

    return 0;
}