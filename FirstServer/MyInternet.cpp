#include "MyInternet.h"
#include"Acceptor.h"
#include"ThreadPool.h"
#include"Handler.h"
#include<unistd.h>

void MyInternet::ProcessDisconnections()
{
	std::lock_guard<std::mutex> lock(epoll_mutex);
	for (int fd : DisconnectList) 
	{
		if (fd == -1) 
			continue;
		if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr) == -1) 
		{
			std::cerr << "Failed to remove file descriptor from epoll: " << std::strerror(errno) << std::endl;
		}
		close(fd);
	}
	DisconnectList.clear();
}
void MyInternet::registerEpoll(int fd, uint32_t events)
{
    epoll_event ev;
	ev.data.fd = fd;
	ev.events = events;
	std::lock_guard<std::mutex> lock(epoll_mutex);
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		std::cerr << "Failed to add file descriptor to epoll: " << std::strerror(errno) << std::endl;
	}
}

void MyInternet::modifyEpoll(int fd, uint32_t events)
{
	epoll_event ev;
	ev.data.fd = fd;
	ev.events = events;
	std::lock_guard<std::mutex> lock(epoll_mutex);

	if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1)
	{
		std::cerr << "Failed to modify epoll events for fd " << fd << ": " << std::strerror(errno) << std::endl;
	}
}

MyInternet::MyInternet()
{
	TheAcceptor = new Acceptor(FIRST_PORT);
	TheThreadPool = new ThreadPool();
	epollfd = epoll_create1(EPOLL_CLOEXEC);
	if (epollfd == -1)
	{
		std::cerr << "Failed to create epoll instance: " << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	registerEpoll(TheAcceptor->GetListenFd(), EPOLLIN);
}

void MyInternet::MainLoop()
{
	epoll_event events[MAX_EVENTS];
	while(true)
	{
		ProcessDisconnections();
		int ready_fds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (ready_fds == -1)
		{
			if (errno == EINTR)
				continue; // Interrupted by signal, retry
			std::cerr << "epoll_wait error: " << std::strerror(errno) << std::endl;
			break;
		}
		for(int i=0;i<ready_fds;++i)
		{
			int fd = events[i].data.fd;
			if(fd==TheAcceptor->GetListenFd())
			{
				TheAcceptor->AcceptConnection(this);
			}
			else if (events[i].events & EPOLLIN)
			{
				//Handle read event
				Handler* NewHandler = new Handler(fd, READING, this,TheThreadPool);
				NewHandler->StartThread();
			}
			else if(events[i].events & EPOLLOUT)
			{
				//Handle write event
				Handler* NewHandler = new Handler(fd, WRITING, this,TheThreadPool);
				NewHandler->StartThread();
			}
		}
	}
}
