#pragma once
#include <errno.h>        
#include<cstring>
#include<iostream>
#include <sys/epoll.h>
#include<vector>
#include<mutex>
#include<set>	

constexpr int FIRST_PORT = 8080;
constexpr int MAX_EVENTS = 1024;
class Acceptor;
class ThreadPool;
class ProcessPool;
// Manages network connections and epoll instance
class MyInternet
{
private:
	std::mutex epoll_mutex;
	std::set<int> DisconnectList;

	Acceptor* TheAcceptor;
	ThreadPool* TheThreadPool;
	ProcessPool* TheProcessPool;
	int epollfd;
	void ProcessDisconnections();
public:
	void registerEpoll(int fd, uint32_t events);
	void modifyEpoll(int fd, uint32_t events);
	MyInternet();
	void MainLoop();

	void RemoveConnection(int fd)
	{
		if (fd == -1) 
			return;
		std::lock_guard<std::mutex> lock(epoll_mutex);
		if(DisconnectList.find(fd) == DisconnectList.end())
			DisconnectList.insert(fd);
	}
};

