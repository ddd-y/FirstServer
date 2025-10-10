#include "Handler.h"
#include"ProcessPool.h"
#include<unistd.h>
#include<string>
#include"MyInternet.h"
#include"metaProcess.h"
#include"ThreadPool.h"

//新增：进入每个if注册Read事件 确保能持续read

void Handler::HandleRead()
{
	char buffer[1024];
	ssize_t bytes_read;
	do 
	{
		bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
	} 
	while (bytes_read == -1 && errno == EINTR); 

	if (bytes_read <= 0)
	{
		// error or connection closed by peer
		TheReactor->RemoveConnection(client_fd);
		return;
	}
	buffer[bytes_read] = '\0';  // be sure to null-terminate
	std::string received_data(buffer, bytes_read);
	std::string command;


	//解决一下不同命令的切割问题
	switch (received_data[0])
	{
	case  'J':
		command = std::move(received_data.substr(0, 4));
		break;
	case 'U':
		command = std::move(received_data.substr(0, 6));
		break;
	case 'R':
		command = std::move(received_data.substr(0, 7));
		break;
	default:
		break;
	}
	//

	
	if (command == CLIENT_REQUEST)
	{	
		
		TheClientStateManager.AddClient(client_fd);
		TheReactor->modifyEpoll(client_fd, EPOLLOUT | EPOLLET);
		return;
	}

	
	if (command == SERVER_JOIN)
	{

		//handle second type server read
		TheProcessPool->AddProcess(getMetaProcessByInfo(client_fd));
		TheReactor->modifyEpoll(client_fd, EPOLLIN | EPOLLET);
		return;
	}

	
	if (command == SERVER_UPDATE)
	{
		//这个协议可能还得改一下
		
		std::string num_str = received_data.substr(6);
		int num_read = num_str.size();
		if (num_read <= 0)
		{
			return;
		}

		try
		{
			int ProcessID = getMetaProcessByInfo(client_fd).relatedfd;
			int update_load = std::stoi(num_str);
			TheProcessPool->UpDateProcessState(ProcessID, update_load);
		}
		catch (const std::invalid_argument&)
		{
			// 非法数字格式
		}
		catch (const std::out_of_range&)
		{
			// 数字超出范围
		}

		TheReactor->modifyEpoll(client_fd, EPOLLIN | EPOLLET);
		return;
	}


	//处理未知命令
	const std::string err_msg = "Invalid command";
	std::cout << err_msg << std::endl;
	write(client_fd, err_msg.c_str(), err_msg.size());
	
}

void Handler::HandleWrite()
{
	if(TheClientStateManager.IsClient(client_fd))
	{
		std::string TheIP = std::move(TheProcessPool->GetProcessIP());
		write(client_fd, TheIP.c_str(), TheIP.size());
		TheClientStateManager.RemoveClient(client_fd);
		TheReactor->RemoveConnection(client_fd);
		TheReactor->modifyEpoll(client_fd, EPOLLIN | EPOLLET);
	}
	else
	{
		//handle second type server write
		return;
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

Handler::Handler(int fd, HandlerState TheState, MyInternet* NewReactor,ThreadPool* NewThreadPool,ProcessPool* NewProcessPool) :client_fd(fd)
, TaskState(TheState), TheReactor(NewReactor),TheThreadPool(NewThreadPool), TheProcessPool(NewProcessPool)
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