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
	std::string data;
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
	if (bytes_read > 0)
	{
		std::cout << "client say :" << buffer << std::endl;
		data.append(buffer, bytes_read - 1);
	}

	

	memset(buffer, 0, sizeof(buffer));
	if(data== CLENT_REQUEST)
	{	
		std::cout << "enter to Adding" << std::endl;
		//handle client request
		TheClientStateManager.AddClient(client_fd);
	}
	else if(data == SERVER_JOIN)
	{	
		std::cout << "enter to registering" << std::endl;
		//handle second type server read
		TheProcessPool->AddProcess(getMetaProcessByInfo(client_fd));
		
	}
	else
	{
		//invalid request
	}

	data = "";
}

void Handler::HandleWrite()
{
	if(TheClientStateManager.IsClient(client_fd))
	{
		std::string TheIP = std::move(TheProcessPool->GetProcessIP());
		write(client_fd, TheIP.c_str(), TheIP.size());
		TheClientStateManager.RemoveClient(client_fd);
		TheReactor->RemoveConnection(client_fd);
		std::cout << "HandleWrite()" << std::endl;
	
	}
	else
	{		//handle second type server write
		std::cout << "HandleWrite()" << std::endl;
		
	}
}

metaProcess Handler::getMetaProcessByInfo(int conn_fd)
{
	struct sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(peer_addr);
	memset(&peer_addr, 0, addr_len);
	int ret = getpeername(conn_fd, (struct sockaddr*)&peer_addr, &addr_len);
	if (ret == -1) 
	{
		return metaProcess();
	}
	uint16_t peer_port = ntohs(peer_addr.sin_port);
	std::string peer_ip = inet_ntoa(peer_addr.sin_addr);
	return metaProcess(conn_fd, peer_ip, peer_port);
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
