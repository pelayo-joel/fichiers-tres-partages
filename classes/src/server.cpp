#include "../include/server.hpp"

Server::Server(int port, int maxQueue) : FTP_Socket(port)
{
    int serverSocket = this->get_socketFD();
    // int enableReuse = 1;

    sinAddress_.sin_addr.s_addr = INADDR_ANY;
    char serverIP[INET_ADDRSTRLEN] = "127.0.0.1";
    inet_ntop(AF_INET, &sinAddress_.sin_addr, serverIP, INET_ADDRSTRLEN);

    // Allows the server socket to connect to multiple client sockets
    // setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&enableReuse, sizeof(enableReuse));

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
        masterFD_ = base.masterFD_;
        activity_ = base.activity_;
        addrlen_ = base.addrlen_;
        maxClients_ = base.maxClients_;
        // clients_ = base.clients_;
        readfds_ = base.readfds_;
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

int Server::getMasterFD() { return masterFD_; }
int Server::getClientFD() { return clientFD_; }
int Server::getActivity() { return activity_; }
int Server::getAddrlen() { return addrlen_; }
int Server::getMaxClients() { return maxClients_; }
// std::vector<int> Server::getClients() { return clients_; }
fd_set Server::getReadfds() { return readfds_; }

void Server::setMasterFD(int masterFD) { masterFD_ = masterFD;}
void Server::setClientFD(int clientFD) { clientFD_ = clientFD;}
void Server::setActivity(int activity) { activity_ = activity; }
void Server::setAddrlen(int addrlen) { addrlen_ = addrlen; }
void Server::setMaxClients(int maxClients) { maxClients_ = maxClients; }
// void Server::setClients(std::vector<int> clients) { clients_ = clients; }
void Server::setReadfds(fd_set readfds) { readfds_ = readfds; }