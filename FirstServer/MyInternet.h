#pragma once
#include <errno.h>        
#include<cstring>
#include<iostream>
#include <sys/epoll.h>
#include<vector>
#include<mutex>

constexpr int FIRST_PORT = 8080;
constexpr int MAX_EVENTS = 1024;
class Acceptor;
class ThreadPool;
// Manages network connections and epoll instance
class MyInternet
{
private:
	int epollfd;
	Acceptor* TheAcceptor;

	std::mutex epoll_mutex;
	std::vector<int> DisconnectList;

	ThreadPool* TheThreadPool;
	void ProcessDisconnections();
public:
	void registerEpoll(int fd, uint32_t events);
	MyInternet();
	void MainLoop();

	void RemoveConnection(int fd)
	{
		std::lock_guard<std::mutex> lock(epoll_mutex);
		DisconnectList.push_back(fd);
	}
};

