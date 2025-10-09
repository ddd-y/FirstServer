#include "Handler.h"
#include"ProcessPool.h"
#include<unistd.h>
#include<string>
#include"MyInternet.h"
#include"metaProcess.h"
#include"ThreadPool.h"
#include"logger.h"

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

	char temp = received_data[0];
	switch (temp)
	{
		case CLIENT_REQUEST[0]:
			HandleCR(std::move(received_data));
			break;
		case SERVER_JOIN[0]:
			HandleSJ(std::move(received_data));
			break;
		case SERVER_UPDATE[0]:
			HandleSU(std::move(received_data));
			break;
		default:
			HandleInvalid();
			break;
	}
}

void Handler::HandleWrite()
{
	if(TheClientStateManager.IsClient(client_fd))
	{
		LOG_DEBUG("Handling client request for fd {}", client_fd);
		std::string TheIP = std::move(TheProcessPool->GetProcessIP());
		if (TheIP == "")
		{
			LOG_DEBUG("client request for fd {} failed to find the min process because there is no ready process", client_fd);
		}
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

void Handler::HandleCR(std::string&& command)
{
	std::string new_command = std::move(command.substr(0,REQUEST_LENGTH));
	if (new_command == CLIENT_REQUEST)
	{
		//handle first type client read
		LOG_DEBUG("Received client request on fd {}", client_fd);
		TheClientStateManager.AddClient(client_fd);
		TheReactor->modifyEpoll(client_fd, EPOLLOUT | EPOLLET);
		return;
	}
}

void Handler::HandleSJ(std::string&& command)
{
	std::string new_command = std::move(command.substr(0, JOIN_LENGTH));
	if (command == SERVER_JOIN)
	{
		//handle second type server read
		LOG_DEBUG("Second type server joining on fd {}", client_fd);
		TheProcessPool->AddProcess(getMetaProcessByInfo(client_fd));
		TheReactor->modifyEpoll(client_fd, EPOLLIN | EPOLLET);
		return;
	}
}

void Handler::HandleSU(std::string&& command)
{
	std::string new_command = std::move(command.substr(0, UPDATE_LENGTH));
	if (command == SERVER_UPDATE)
	{
		std::string num_str = std::move(command.substr(UPDATE_LENGTH));
		int num_read = num_str.size();
		if (num_read <= 0)
		{
			return;
		}
		int ProcessID = getMetaProcessByInfo(client_fd).relatedfd;
		try
		{
			int update_load = std::stoi(num_str);
			LOG_DEBUG("Updating process load: fd {}, new load {}", ProcessID, update_load);
			TheProcessPool->UpDateProcessState(ProcessID, update_load);
			TheReactor->modifyEpoll(client_fd, EPOLLIN | EPOLLET);
		}
		catch (const std::invalid_argument&)
		{
			// 非法数字格式
			LOG_DEBUG("Updating process load: fd {}, because of invalid argument", ProcessID);
		}
		catch (const std::out_of_range&)
		{
			// 数字超出范围
			LOG_DEBUG("Updating process load: fd {}, because of the number out of range", ProcessID);
		}
		return;
	}
}

void Handler::HandleInvalid()
{
	//处理未知命令
	const std::string err_msg = "Invalid command";
	write(client_fd, err_msg.c_str(), err_msg.size());
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

Handler::Handler(int fd, HandlerState TheState, MyInternet* NewReactor,ThreadPool* NewThreadPool, 
	ProcessPool* NewProcessPool) :client_fd(fd), TheProcessPool(NewProcessPool)
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
