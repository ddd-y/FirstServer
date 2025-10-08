#include "Handler.h"
#include"ProcessPool.h"
#include<unistd.h>
#include<string>
#include"MyInternet.h"
#include"metaProcess.h"
#include"ThreadPool.h"
#include <stddef.h>  
#include <stdexcept>

void Handler::HandleRead()
{
	/*char buffer[1024];
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	std::string data;
	data.append(buffer, bytes_read);
	if(data== CLENT_REQUEST)
	{
		//handle client request
		TheClientStateManager.AddClient(client_fd);
	}
	else if(data == SERVER_JOIN)
	{
		//handle second type server read
		TheProcessPool->AddProcess(getMetaProcessByInfo(client_fd));
	}
	else
	{
		//invalid request
	}*/
    // 1. 读取数据并处理基础错误
    char buffer[1024];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0)
    {
        // error or connection closed by peer
        return;
    }
    buffer[bytes_read] = '\0';  // be sure to null-terminate
    std::string received_data(buffer, bytes_read);

    // 提取消息（基于分隔符拆分，解决粘包）
    size_t delim_pos = received_data.find(MSG_DELIMITER);
    if (delim_pos == std::string::npos)
    {

        return;
    }
    std::string command = received_data.substr(0, delim_pos);

    if (command == CLIENT_REQUEST)
    {
        TheClientStateManager.AddClient(client_fd);
        return;
    }

    if (command == SERVER_JOIN)
    {

        //handle second type server read
        TheProcessPool->AddProcess(getMetaProcessByInfo(client_fd));
        return;
    }

    if (command == SERVER_UPDATE)
    {
        char num_buffer[32];
        ssize_t num_read = read(client_fd, num_buffer, sizeof(num_buffer) - 1);
        if (num_read <= 0)
        {
            return;
        }
        num_buffer[num_read] = '\0';
        std::string num_str(num_buffer);
        size_t num_delim = num_str.find(MSG_DELIMITER);

        if (num_delim == std::string::npos)
        {
            // 数字格式不完整
            return;
        }

       
        try
        {
            ProcessPool->UpDateProcessState();
        }
        catch (const std::invalid_argument&)
        {
            // 非法数字格式
        }
        catch (const std::out_of_range&)
        {
            // 数字超出范围
        }
        return;
    }

    // 4. 处理未知命令
    const std::string err_msg = "Invalid command" + MSG_DELIMITER;
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
