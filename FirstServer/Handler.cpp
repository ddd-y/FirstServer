#include "Handler.h"
#include"ProcessPool.h"
#include<unistd.h>
#include<string>
#include"MyInternet.h"
#include"metaProcess.h"
#include"ThreadPool.h"

void Handler::HandleRead()
{
	char buffer[1024];
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	std::string data;
	data.append(buffer, bytes_read);
	if(data== CLENT_REQUEST)
	{
		//handle client request
	}
	else if(data == SERVER_JOIN)
	{
		//handle second type server read
	}
	else
	{
		//invalid request
	}
}

void Handler::HandleWrite()
{
	if(TheClientStateManager.IsClient(client_fd))
	{
		std::string TheIP = std::move(TheProcessPool->GetProcessIP());
		write(client_fd, TheIP.c_str(), TheIP.size());
		TheClientStateManager.RemoveClient(client_fd);
		TheReactor->RemoveConnection(client_fd);
	}
	else
	{
		//handle second type server write
		return;
	}
}

std::string Handler::getPeerIP(int conn_fd, int& getport)
{
	struct sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(peer_addr);
	std::memset(&peer_addr, 0, addr_len);

	if (getpeername(conn_fd, (struct sockaddr*)&peer_addr, &addr_len) == -1) {
		std::cerr << "getpeername failed: " << std::strerror(errno) << std::endl;
		return ""; 
	}
	if (peer_addr.sin_family != AF_INET) {
		std::cerr << "Unsupported address family (not IPv4)" << std::endl;
		return "";
	}

	char ip_str[INET_ADDRSTRLEN]; 
	if (inet_ntop(AF_INET, &(peer_addr.sin_addr), ip_str, sizeof(ip_str)) == nullptr) {
		std::cerr << "inet_ntop failed: " << std::strerror(errno) << std::endl;
		return "";
	}
	getport = ntohs(peer_addr.sin_port);
	return std::string(ip_str); 
}

Handler::Handler(int fd, HandlerState TheState, MyInternet* NewReactor,ThreadPool* NewThreadPool) :client_fd(fd)
, TaskState(TheState), TheReactor(NewReactor),TheThreadPool(NewThreadPool)
{
	if (TaskState == READING)
	{
		CurrentHandle = &Handler::HandleRead;
	}
	else
	{
		CurrentHandle = &Handler::HandleWrite;
	}
}

void Handler::StartThread()
{
	TheThreadPool->AddTask(TaskData{ client_fd,this });
}

void Handler::Handle()
{
	(this->*CurrentHandle)();
}
