#include "Acceptor.h"
#include <sys/socket.h>
#include<cstring>
#include"MyInternet.h"
Acceptor::Acceptor(int port)
{
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    //start listening
    listen(listenfd, SOMAXCONN);
}

void Acceptor::AcceptConnection(MyInternet* TheReactor)
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
    if (connfd == -1)
    {
        std::cerr << "Accept error: " << std::strerror(errno) << std::endl;
        return;
    }
    // Register the new connection with epoll for read events
	TheReactor->registerEpoll(connfd, EPOLLIN | EPOLLET);
}
