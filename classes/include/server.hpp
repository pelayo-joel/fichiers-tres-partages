#pragma once

#include <pthreads>
#include <queue>

#include "FTP_Socket.hpp"


class Server : public FTP_Socket
{
private:
    int masterFD_, clientFD_, activity_, addrlen_, maxClients_;
    std::queue<pthread_t> clientsThread_;
    fd_set readfds_;

    int bind();
    int listen(int maxQueue);

public:
    Server(int port, int maxQueue);
    Server(const Server &base);
    ~Server();
    Server &operator=(const Server &base);

    int accept();
    int getMasterFD();
    int getClientFD();
    int getActivity();
    int getAddrlen();
    int getMaxClients();
    std::queue<int> getClients();
    fd_set getReadfds();

    void setMasterFD(int masterFD);
    void setClientFD(int clientFD);
    void setActivity(int activity);
    void setAddrlen(int addrlen);
    void setMaxClients(int maxClients);
    // void setClients(std::vector<int> clients);
    void setReadfds(fd_set readfds);
};