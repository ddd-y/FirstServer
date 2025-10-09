#include "MyInternet.h"
#include"Acceptor.h"
#include"ThreadPool.h"
#include"Handler.h"
#include"ProcessPool.h"
#include<unistd.h>
#include"logger.h"

void MyInternet::ProcessDisconnections()
{
	std::lock_guard<std::mutex> lock(epoll_mutex);
	for (int fd : DisconnectList) 
	{
		if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr) == -1) 
		{
			LOG_ERROR("Failed to remove fd {} from epoll: {}", fd, std::strerror(errno));
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
		LOG_ERROR("Failed to add fd {} to epoll: {}", fd, std::strerror(errno));
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
		LOG_ERROR("Failed to modify epoll events for fd {}: {}", fd, std::strerror(errno));
	}
}

MyInternet::MyInternet()
{
	TheAcceptor = new Acceptor(FIRST_PORT);
	TheThreadPool = new ThreadPool();
	TheProcessPool = new ProcessPool();
	epollfd = epoll_create1(EPOLL_CLOEXEC);
	if (epollfd == -1)
	{
		LOG_ERROR("Failed to create epoll instance: {}", std::strerror(errno));
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
			LOG_ERROR("epoll_wait error: {}", std::strerror(errno));
			break;
		}
		for(int i=0;i<ready_fds;++i)
		{
			int fd = events[i].data.fd;
			epoll_mutex.lock();
			if(DisconnectList.find(fd) != DisconnectList.end())
			{
				// Skip processing for this fd
				continue; 
			}
			epoll_mutex.unlock();
			if(fd==TheAcceptor->GetListenFd())
			{
				TheAcceptor->AcceptConnection(this);
			}
			else if (events[i].events & EPOLLIN)
			{
				//Handle read event
				Handler* NewHandler = new Handler(fd, READING, this,TheThreadPool,TheProcessPool);
				NewHandler->StartThread();
			}
			else if(events[i].events & EPOLLOUT)
			{
				//Handle write event
				Handler* NewHandler = new Handler(fd, WRITING, this,TheThreadPool,TheProcessPool);
				NewHandler->StartThread();
			}
		}
	}
}
