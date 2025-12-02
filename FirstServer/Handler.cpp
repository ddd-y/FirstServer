#include "Handler.h"
#include"ProcessPool.h"
#include<unistd.h>
#include<string>
#include"MyInternet.h"
#include"metaProcess.h"
#include"ThreadPool.h"
#include"logger.h"

bool Handler::HandleRead()
{
	LastHandleTime = std::chrono::steady_clock::now();
	std::string received_data=IncompleteMes;
	char buffer[1024];
	while (true) 
	{
		ssize_t bytes_read= read(client_fd, buffer, sizeof(buffer) - 1);
		if (bytes_read == -1)
		{
			if (errno == EINTR) 
			{
				continue;
			}
			else if (errno == EAGAIN || errno == EWOULDBLOCK) 
			{
				break;
			}
			else 
			{
				LOG_ERROR("Read error for fd {}: {}", client_fd, std::strerror(errno));
				CleanupConnection();
				return true;
			}
		}
		else if (bytes_read == 0)
		{
			// 客户端关闭连接且未发送 ";"，视为无效消息
			LOG_WARN("Client fd {} closed without '.'", client_fd);
			CleanupConnection();
			return true;
		}
		received_data.append(buffer, bytes_read);
		size_t semicolon_pos = received_data.find(PARTITION_CHAR);
		if (semicolon_pos != std::string::npos)
		{
			std::string complete_msg = received_data.substr(0, semicolon_pos + 1);
			HandleCompleteMes(complete_msg);
			return true;
		}
	}
	if (!received_data.empty()) 
	{
		if (received_data.size() >= MAX_INCOMPMES_SIZE)
		{
			LOG_DEBUG("Incomplete message for fd {}, is too large", client_fd);
		}
		IncompleteMes = received_data;
		LOG_DEBUG("Incomplete message for fd {}, waiting for more data", client_fd);
		TheReactor->modifyEpoll(client_fd, EPOLLIN | EPOLLET);
		return false;
	}
	else 
	{
		LOG_WARN("No data received for fd {}", client_fd);
		CleanupConnection();
		return true;
	}
	return true;
}

bool Handler::HandleWrite()
{
	LastHandleTime = std::chrono::steady_clock::now();
	std::string data_to_write;

	// 优先处理未完成的写数据
	if (!IncompleteMes.empty()) 
	{
		data_to_write = IncompleteMes;
		IncompleteMes.clear();
	}
	else 
	{
		// 根据客户端类型准备要发送的数据
		if (TheClientStateManager.IsClient(client_fd)) 
		{
			LOG_DEBUG("Handling client request for fd {}", client_fd);
			std::string TheIP = TheProcessPool->GetProcessIP();
			if (TheIP.empty()) 
			{
				LOG_DEBUG("client request for fd {} failed: no ready process", client_fd);
				data_to_write = "NoReadyServer";
			}
			else 
			{
				data_to_write = TheIP;
			}
		}
		else 
		{
			// 服务器类型连接：发送加入成功消息
			data_to_write = "SuccessJoin";
		}
	}
	// 执行写操作
	ssize_t total_written = 0;
	const char* buffer = data_to_write.c_str();
	size_t remaining = data_to_write.size();
	while (remaining > 0) 
	{
		ssize_t bytes_written = write(client_fd, buffer + total_written, remaining);
		if (bytes_written == -1) 
		{
			if (errno == EINTR) 
			{
				// 被信号中断，重试
				continue;
			}
			else if (errno == EAGAIN || errno == EWOULDBLOCK) 
			{
				// 内核缓冲区满，保存未写完的数据，等待下次EPOLLOUT事件
				IncompleteMes = data_to_write.substr(total_written);
				LOG_DEBUG("Partial write for fd {}, remaining: {} bytes", client_fd, remaining);
				TheReactor->modifyEpoll(client_fd, EPOLLOUT | EPOLLET);
				return false; // 未完成，保留handler
			}
			else 
			{
				LOG_ERROR("Write error for fd {}: {}", client_fd, std::strerror(errno));
				CleanupConnection();
				return true;
			}
		}
		else if (bytes_written == 0) 
		{
			LOG_WARN("Client fd {} closed during write", client_fd);
			CleanupConnection();
			return true;
		}

		total_written += bytes_written;
		remaining -= bytes_written;
	}
	//handle之后的处理
	if (TheClientStateManager.IsClient(client_fd)) 
	{
		CleanupConnection();
	}
	else 
	{
		TheReactor->modifyEpoll(client_fd, EPOLLIN | EPOLLET);
	}
	return true;
}

void Handler::HandleCR(std::string& command)
{
	std::string new_command = command.substr(0, REQUEST_LENGTH);
	if (new_command == CLIENT_REQUEST)
	{
		//handle first type client read
		LOG_DEBUG("Received client request on fd {}", client_fd);
		TheClientStateManager.AddClient(client_fd);
		TheReactor->modifyEpoll(client_fd, EPOLLOUT | EPOLLET);
		return;
	}
}

void Handler::HandleSJ(std::string& command)
{
	std::string new_command = command.substr(0, JOIN_LENGTH);
	if (new_command == SERVER_JOIN)
	{
		//handle second type server read
		LOG_DEBUG("Second type server joining on fd {}", client_fd);
		TheProcessPool->AddProcess(getMetaProcessByInfo(client_fd));
		LOG_DEBUG("Server Join IP:{}", TheProcessPool->GetProcessIP());

		TheReactor->modifyEpoll(client_fd, EPOLLOUT | EPOLLET);
		return;
	}
}

void Handler::HandleSU(std::string& command)
{
	std::string new_command = command.substr(0, UPDATE_LENGTH);
	if (new_command == SERVER_UPDATE)
	{
		std::string num_str = command.substr(UPDATE_LENGTH);
		int num_read = num_str.size();
		if (num_read <= 0)
		{
			return;
		}
		int ProcessID = client_fd;
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

void Handler::HandleSL(std::string& command)
{
	std::string new_command = command.substr(0, LEAVE_LENGTH);
	if(new_command==SERVER_LEAVE)
	{
		int ProcessID = client_fd;
		TheProcessPool->RemoveProcess(ProcessID);
		TheReactor->RemoveConnection(ProcessID);
	}
}

void Handler::HandleFE(std::string& command)
{
	std::string new_command = command.substr(0, FIND_ERROR_LENGTH);
	if (new_command == CLIENT_FIND_ERROR)
	{
		std::string WrongIP = command.substr(FIND_ERROR_LENGTH);
		//后边写一写通过IP来找连接的代码
		TheProcessPool->HandleClientError(WrongIP);
	}
}

void Handler::HandleInvalid()
{
	//处理未知命令
	const std::string err_msg = "Invalid command";
	write(client_fd, err_msg.c_str(), err_msg.size());
	TheClientStateManager.RemoveClient(client_fd);
	TheReactor->RemoveConnection(client_fd);
}

void Handler::CleanupConnection()
{
	LOG_DEBUG("Cleaning up connection fd: {}", client_fd);
	if (!TheClientStateManager.IsClient(client_fd))
		TheProcessPool->RemoveProcess(client_fd);
	else
		TheClientStateManager.RemoveClient(client_fd);
	TheReactor->RemoveConnection(client_fd);
}

void Handler::HandleCompleteMes(std::string& command)
{
	char temp = command[0];
	switch (temp)
	{
	case CLIENT_REQUEST[0]:
		HandleCR(command);
		break;
	case CLIENT_FIND_ERROR[0]:
		HandleFE(command);
		break;
	case SERVER_JOIN[0]:
		HandleSJ(command);
		break;
	case SERVER_UPDATE[0]:
		HandleSU(command);
		break;
	case SERVER_LEAVE[0]:
		HandleSL(command);
		break;
	default:
		HandleInvalid();
		break;
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



Handler::Handler(int fd, HandlerState TheState):
	LastHandleTime(std::chrono::steady_clock::now()), IncompleteMes(""), client_fd(fd), TaskState(TheState), 
	TheProcessPool(ProcessPool::getInstance()),
    TheReactor(MyInternet::getInstance())
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

bool Handler::Handle()
{
	bool result=(this->*CurrentHandle)();
	return result;
}

bool Handler::IfTooLong(std::chrono::steady_clock::time_point& current_time)
{
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - LastHandleTime);
	return duration.count() > MAX_LIFE;
}

