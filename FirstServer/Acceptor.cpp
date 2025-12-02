#include "Acceptor.h"
#include <sys/socket.h>
#include<cstring>
#include"MyInternet.h"
#include"logger.h"

Acceptor::Acceptor(int port)
{
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) 
    {
        LOG_ERROR("socket create error: {}", std::strerror(errno));
    }
    int flags = fcntl(listenfd, F_GETFL, 0);
    if (flags == -1 || fcntl(listenfd, F_SETFL, flags | O_NONBLOCK) == -1) 
    {
        LOG_ERROR("fcntl set listenfd nonblock error: {}", std::strerror(errno));
        close(listenfd);
    }
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    //start listening
    listen(listenfd, SOMAXCONN);
	LOG_INFO("Server listening on port {}", port);
}

void Acceptor::AcceptConnection(MyInternet* TheReactor)
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (true) 
    {
        int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
        if (connfd == -1) 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                LOG_DEBUG("No more new connections to accept");
                break;
            }
            else if (errno == EINTR) 
            {
                LOG_DEBUG("accept interrupted by signal, retrying");
                continue;
            }
            else 
            {
                LOG_ERROR("Accept error: {}", std::strerror(errno));
                break;
            }
        }
        int flags = fcntl(connfd, F_GETFL, 0);
        if (flags == -1) 
        {
            LOG_ERROR("fcntl get error for fd {}: {}", connfd, std::strerror(errno));
            close(connfd);
            continue; 
        }
        if (fcntl(connfd, F_SETFL, flags | O_NONBLOCK) == -1) 
        {
            LOG_ERROR("fcntl set nonblock error for fd {}: {}", connfd, std::strerror(errno));
            close(connfd);
            continue; 
        }
        TheReactor->registerEpoll(connfd, EPOLLIN | EPOLLET);
        LOG_DEBUG("New connection accepted, fd: {}", connfd);
    }
}
